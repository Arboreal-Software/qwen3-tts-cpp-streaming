# CUDA candidate runtime failure — 2026-10-27

The generation profiling build compiled successfully with QWEN3_TTS_TIMING=ON,
then aborted with CUDA illegal memory access during normal asynchronous streaming.
The last completed window was index9, cumulative PCM7016.9ms at wall4584ms.
Both transformer and decoder threads reported errors at CUDA stream synchronization.
This identifies observation points, not the offending kernel or root cause.

Raw evidence: results/generation-profile.log. Container dax-generation-profile;
original candidate source/build, opt-in FP32 transpose enabled, same sentence/seed
as prior benchmark. Prior successful runs do not establish stability. Timing
instrumentation changes scheduling; do not conclude that it caused the defect.

Deployment is blocked pending diagnosis. Preserve independent baseline. Next:
run with CUDA_LAUNCH_BLOCKING=1 or compute-sanitizer to localize the first failing
operation; compare asynchronous versus synchronous decode and flags independently;
check GPU health and reproduce without instrumentation. Keep numerical and speed
results as historical measurements, not readiness claims.
