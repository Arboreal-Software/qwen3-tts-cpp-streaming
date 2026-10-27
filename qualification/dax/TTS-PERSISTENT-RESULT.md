# Persistent Qwen3-TTS reuse and cancellation — preliminary failure

Actual Dax GTX1080, isolated candidate library, one loaded engine, deterministic seed42, unpaced callbacks. Probe sequence: same sentence twice, different long sentence cancelled after first audio callback, original sentence again. Source tts-persistent-probe.cpp; run-persistent-probe.sh. This is a library test, not network/UI interruption qualification.

First run: engine load1636ms; normal requests7113/6993ms; cancellation returned67ms after callback rejected first chunk, with zero subsequent callbacks. Same-seed original sentence after cancellation completed but failed exact PCM equality.

Second run with PCM capture reproduced failure: first two requests bit-identical; cancellation returned31ms after rejection, zero subsequent callbacks; post-cancel response same158805 samples but49920 samples differ, max absolute float difference0.82426238 and RMS0.05535776. These are not small rounding differences. Exit4 is the deliberate equality-test failure, not a container crash. Do not qualify transparent persistent-engine reuse after cancellation.

Cause not established. Need control with the intervening different sentence completed rather than cancelled, inspect actual audio-code sequences and differences by chunk, and compare original engine versus candidate. Stateful transformer/decoder/graph reuse, history-dependent behavior and cancellation-specific cleanup remain hypotheses. A per-session process boundary and explicit retirement/reload after cancellation may contain risk but would add latency; not yet selected or measured. No live service changed.

Decoder wall profile also shows compute overwhelmingly dominates graph allocation/upload/download in the baseline. Separate cumulative decoder-stage probe queued in server-setup:decoder-stages to identify the heavy block; profile prefixes are diagnostic and require careful interpretation of subtraction noise and warmup.

## Completed-intervening-request control

The same different sentence was allowed to complete instead of cancelling. It completed successfully; the following original prompt had exactly the same difference metrics as both cancellation runs:49920 samples differ, max_abs0.8242623806,RMS0.0553577611763. Difference indices108885..158804 correspond only to the final2.080s; first4.537s are bit-identical. Therefore cancellation is NOT a necessary trigger. This is a history-dependent reproducibility observation, not yet proof of wrong speech, semantic leakage or a cleanup bug. Same-seed output can diverge through changed numerical execution paths; establish code/token and spoken-content differences before requiring bitwise identity as a serving gate. Updated probe separates intermediate-request comparison from reference equality.

Next controls: inspect generated code sequences and attention/cache allocation across different prompt lengths; compare spoken content; test clean request cache reinitialization without reloading model weights if needed. Do not claim cancellation repair is required on the current evidence alone.
