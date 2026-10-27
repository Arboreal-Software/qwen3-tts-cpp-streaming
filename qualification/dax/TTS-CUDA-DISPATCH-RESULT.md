# TTS CUDA dispatch qualification — 2026-10-27

## Finding

The original decoder placed its transposed convolutions on CPU: its weights are F16, but this GGML CUDA backend accepts only F32 inputs and weights for CONV_TRANSPOSE_1D. Overall CUDA initialization did not establish GPU placement for these operators.

The disposable candidate adds opt-in graph casts with DAX_CUDA_TRANSPOSE_F32=1. Operator instrumentation now reports CUDA0 for these nodes. It also contains the previously tested bounded-loop CUDA patch. Neither change is deployed to a live voice service.

## First measured comparison

Same pinned model, speaker embedding, prompt, seed and Docker limits. Each output is 11.497 seconds of 24kHz mono audio.

| Case | Synthesis | First PCM | Maximum window gap |
|---|---:|---:|---:|
| Original | 12.131s | 814ms | 902ms |
| CUDA candidate | 7.562s | 336ms | 516ms |
| Candidate overlap A | 14.401s | 617ms | 995ms |
| Candidate overlap B | 14.438s | 619ms | 1000ms |

One paired sample: 1.60x solo speedup. Two simultaneous processes remain slower than real time. CLI code-generation timing is residual wall time, not an isolated model kernel measurement; do not infer a model regression solely from that field.

The candidate solo and both overlapping outputs are byte-identical to one another. They differ slightly from the CPU baseline: normalized RMS error 0.00003948, maximum absolute error 0.00238, same sample count, no clipped candidate samples. This is consistent with arithmetic differences but does not replace listening/intelligibility acceptance.

Evidence: results/tts-cuda-transpose-20261027T053401Z/{summary.json,waveform-comparison.json}, results/cuda-transpose.log and cuda-transpose-source.patch.

## Follow-up

An otherwise matched run with delivery pacing disabled is in progress in Dax server-setup:cuda-unpaced. Initial solo result is 7.524s vs 11.910s original, suggesting pacing does not explain the remaining cost. Collect overlapping results before deciding next optimization.

Continue profiling actual remaining operators, CPU scheduling and code generation. Requalify cancellation, repeated requests and isolation with the candidate before persistent deployment. Two live users, physical playback, ASR coexistence and long-running memory stability remain unqualified.

Unpaced control completed: original 11.910s, candidate 7.524s, overlapping 14.393/14.429s. Candidate WAV hashes match the paced candidate. Delivery pacing is not the significant bottleneck in these measurements. Next diagnostic widens each container CPU quota from two to four logical CPU equivalents, holding other settings fixed; this is a diagnosis, not an allocation recommendation for ASR coexistence.
