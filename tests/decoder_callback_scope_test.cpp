#include "decoder/scoped_eval_callback.h"
#include <stdexcept>

struct ggml_backend_sched {
    ggml_backend_sched_eval_callback callback = nullptr;
    void * data = nullptr;
    int registrations = 0;
    int removals = 0;
};

// Link the production guard to a recording scheduler, with no model or GPU.
void ggml_backend_sched_set_eval_callback(ggml_backend_sched_t sched,
        ggml_backend_sched_eval_callback callback, void * data) {
    sched->callback = callback;
    sched->data = data;
    if (callback) ++sched->registrations;
    else ++sched->removals;
}

static void require(bool condition) {
    if (!condition) throw std::runtime_error("callback ownership invariant failed");
}
static bool observe(ggml_tensor *, bool, void * data) {
    ++*static_cast<int *>(data);
    return true;
}
static void compute(ggml_backend_sched_t sched, bool throws) {
    int stack_timer = 0;
    qwen3_tts::decoder_internal::ScopedEvalCallback guard(sched, observe, &stack_timer, true);
    require(sched->callback && sched->data == &stack_timer);
    sched->callback(nullptr, false, sched->data);
    require(stack_timer == 1);
    if (throws) throw 42;
}

int main() {
    ggml_backend_sched a, b;
    compute(&a, false);
    require(!a.callback && !a.data && a.removals == 1);
    try { compute(&a, true); return 1; } catch (int value) { require(value == 42); }
    require(!a.callback && !a.data && a.registrations == 2 && a.removals == 2);
    int other_timer = 0;
    {
        qwen3_tts::decoder_internal::ScopedEvalCallback other(&b, observe, &other_timer, true);
        compute(&a, false);
        require(b.callback && b.data == &other_timer && b.removals == 0);
        {
            qwen3_tts::decoder_internal::ScopedEvalCallback disabled(&b, observe, nullptr, false);
        }
        require(b.callback && b.data == &other_timer && b.registrations == 1 && b.removals == 0);
    }
    require(!b.callback && !b.data && b.removals == 1);
}
