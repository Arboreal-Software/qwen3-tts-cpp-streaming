#ifdef NDEBUG
#undef NDEBUG
#endif
#include "transformer/sampling_safety.h"
#include <cassert>
#include <limits>
#include <initializer_list>
using namespace qwen3_tts;
int main() {
    const float nan=std::numeric_limits<float>::quiet_NaN(),inf=std::numeric_limits<float>::infinity();
    float normal[]={-2.0f,1.0f,-inf};assert(sampling_logits_valid(normal,3));
    for(float bad:{nan,inf}) {float values[]={1,bad,2};assert(!sampling_logits_valid(values,3));}
    float absent[]={-inf,-inf};assert(!sampling_logits_valid(absent,2));
    assert(!sampling_logits_valid(nullptr,3));assert(!sampling_logits_valid(normal,0));
    float probabilities[]={.25f,.5f,0};assert(sampling_probabilities_valid(probabilities,3));
    for(float bad:{nan,inf,-.1f}) {float values[]={.5f,bad};assert(!sampling_probabilities_valid(values,2));}
    float zero[]={0,0};assert(!sampling_probabilities_valid(zero,2));
    // Surviving nucleus probabilities need not sum to one: discrete_distribution
    // normalizes them. The safety guard must preserve valid existing sampling.
    float nucleus[]={.2f,0,0};assert(sampling_probabilities_valid(nucleus,3));
}
