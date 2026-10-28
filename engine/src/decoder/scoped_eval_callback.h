#pragma once

#include "ggml-backend.h"

namespace qwen3_tts::decoder_internal {

// Decoder scheduler is exclusively owned during decode. The callback borrows
// stack data and must not outlive the compute scope, including C++ unwinding.
// This does not recover an aborted process or a poisoned CUDA context.
class ScopedEvalCallback {
public:
    ScopedEvalCallback(ggml_backend_sched_t sched,
                       ggml_backend_sched_eval_callback callback, void * data,
                       bool enabled) : sched_(enabled ? sched : nullptr) {
        if (sched_) ggml_backend_sched_set_eval_callback(sched_, callback, data);
    }
    ~ScopedEvalCallback() noexcept {
        if (sched_) ggml_backend_sched_set_eval_callback(sched_, nullptr, nullptr);
    }
    ScopedEvalCallback(const ScopedEvalCallback &) = delete;
    ScopedEvalCallback & operator=(const ScopedEvalCallback &) = delete;
private:
    ggml_backend_sched_t sched_;
};

} // namespace qwen3_tts::decoder_internal
