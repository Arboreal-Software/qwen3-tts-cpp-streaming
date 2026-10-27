#pragma once
#include <cmath>
#include <cstdint>
namespace qwen3_tts {
// Negative infinity is an intentional vocabulary mask. NaN/+inf and an empty
// support must never reach sort/discrete_distribution or masquerade as EOS.
inline bool sampling_logits_valid(const float * values, int32_t count) {
    if (!values || count <= 0) return false;
    bool finite = false;
    for (int32_t i = 0; i < count; ++i) {
        if (std::isnan(values[i]) || values[i] == INFINITY) return false;
        finite = finite || std::isfinite(values[i]);
    }
    return finite;
}
inline bool sampling_probabilities_valid(const float * values, int32_t count) {
    if (!values || count <= 0) return false;
    double total = 0.0;
    for (int32_t i = 0; i < count; ++i) {
        if (!std::isfinite(values[i]) || values[i] < 0.0f) return false;
        total += values[i];
    }
    return std::isfinite(total) && total > 0.0;
}
}
