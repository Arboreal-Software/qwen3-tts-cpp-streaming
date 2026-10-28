# Arboreal engine development

Maintain streaming Qwen3-TTS engine changes here. Keep orchestration, Pi tools,
browser audio and deployment control in separate repositories.

- Preserve upstream history, Apache-2.0 license and vendored GGML notices.
- Work on development branches. Pin deployments by exact qualified commit.
- Do not change active Dax checkout/build directories or restart services to reorganize code.
  Dax operations must run in its server-setup tmux session.
- Keep compatibility fixes, instrumentation and experimental performance changes separately
  reviewable. Experimental flags remain opt-in.
- Preserve successive PCM callbacks, bounded interruption and request isolation.
  First audio latency alone is not successful streaming.
- Test actual placement, waveform/reference behavior, chunk gaps, reuse, cancellation and
  memory growth. Never infer two-user readiness from solo speed.
- Keep model weights, recordings, credentials, build outputs and benchmark archives out
  of Git. Publish compact reports with provenance.
- The copied CUDA reference test has a source hash guard; update coverage when changing
  the production kernel. A copied kernel test alone does not test scheduler dispatch.
- Respect SM61 and Ivy Bridge constraints: no assumed AVX2/FMA/BMI2, tensor cores or fast
  FP16 arithmetic. Keep host dependencies minimal and builds containerized.
- History-dependent waveform variation is unresolved, not proof of cancellation corruption.
  Verify spoken content and isolation before conclusions.

Owner clarification (2026-10-27): exact waveform equality is not required and
history-dependent variation alone is not a deployment blocker. Prioritize spoken
content, perceptual quality, cross-user isolation and repeated/interrupted request
stability. If those pass, document and accept variation. Keep CUDA crash/recovery
qualification independent; good audio does not prove runtime safety.
