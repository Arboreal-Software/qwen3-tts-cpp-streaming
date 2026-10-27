# Opt-in persistent-waveform localization

`ARBOREAL_TRACE_WINDOWS=/existing/private/directory` dumps each decoded window's
exact codec inputs (`window-N.codes`) and decoder output before trimming/crossfade
(`window-N.pcm`). Disabled when unset/empty. Synthesis captures the path once.
Each file begins with five native-endian int32 values: context-start frame,
new-start frame, end frame, number of codebooks, final flag. Remaining payload is
int32 codec IDs or float32 PCM. Qualification currently targets little-endian Dax.
Write failure fails synthesis; no automatic retry. A new directory per request is
required operationally to avoid overwriting evidence. Do not enable on private
voice traffic: these files contain speech data. They must not be committed.

The persistent qualification executable accepts DAX_PROBE_TRACE_ROOT, creates a
new turn-N directory for each request, and sets the engine diagnostic path. Its
existing A/A/B/A sequence and cancellation control are unchanged. Trace overhead
changes timing; do not use these runs as performance measurements.

Compare A turn0 with A turn3 window metadata and codec IDs first. If equal,
compare the same pre-trim decoder PCM. If both equal, investigate trimming/
crossfade/callback assembly. If codes differ, investigate generation state before
blaming the decoder. Cancelled B versus completed B remain separate controls.

Both modified translation units pass C++17 syntax checking. Full Dax build and
actual trace comparison remain pending; no live service uses this diagnostic.

For generation localization, the probe also accepts DAX_PROBE_CAPTURE_DEBUG=1
with an explicit QWEN3_TTS_DEBUG_DUMP_DIR and DAX_PROBE_TRACE_ROOT. It moves the
upstream debug directory into each completed turn's generation/ directory, then
recreates the original empty directory. This avoids overwriting prior-turn
logits while preserving the upstream process-static debug config. No active
writer remains when the move happens. Use a dedicated test output filesystem;
no existing target is overwritten. Request e.g. MAX_FRAMES=58/MAX_CODE_STEPS=2
to include the first differing residual code at frame57/codebook2. This remains
public-fixture-only diagnostics, not production logging or performance evidence.
