# Worker speech duration budget

## Superseding five-minute qualification

The deployed 30-second worker ceiling described below was insufficient: two
technical speech requests terminated after 29.976875 seconds of generated audio.
Raising only the outer voice delivery allowance did not remove that inner limit.
The worker now allows 3750 codec frames and 7,200,000 emitted 24 kHz samples.
Natural EOS, cancellation fencing, bounded pipe writes, and explicit failure
without replay remain unchanged. Service, supervisor and client request bounds
were separately aligned to 300 seconds; the client's default queued-audio budget
remains 30 seconds.

On Dax's GTX1080, the new worker/service/client completed a 30.776875-second
single sentence and a 92.330625-second three-sentence request. The latter took
60.269 seconds to synthesize, first audio 398.6 ms, maximum inter-chunk gap
524.3 ms. A technical filename/hash fixture completed in 13.416875 seconds of
audio. Cancellation settled in 336 ms and two subsequent requests completed.
These are synthetic-text transport/generation results, not human listening or
pronunciation acceptance. Five full minutes of real speech was not generated.

All 12 native protocol tests pass, including synthetic audio beyond 30 seconds,
whole-frame rejection at the 300-second cap and cancellation/reuse. The actual
Python3.10 service image passed 52 tests. Initial assertion/timeout-compatibility
failures were corrected and retained in the qualification records.

Qualified worker SHA256:
`0d3b9231fcba32a92937c9ce90a09d5c011d9bb5dd5f1f9f789eae3b4147c1a2`.
Service image:
`sha256:ec66596174908d8541cb0f7f4cd109d67e98320656a1e9c5e804b1f7efb506f0`.
Evidence: host setup repository `hosts/dax/qualification/tts-five-minute-v1/`,
including collected real-GPU results and deployment receipt. Active Dax serving
uses `~/voice-services/qualification/tts-five-minute-v1/build`; original worker,
image and stopped container are retained for rollback. Model and Lana voice
selection are unchanged.

## Historical 30-second qualification

The persistent worker's upstream five-audio-frames-per-text-token heuristic
truncated an ordinary sentence at 70 frames, before natural EOS. Fresh and
post-cancellation requests both reproduced it, so cancellation is not a necessary
cause. Public fixture: “A new request can begin after cancellation. This one
should finish normally.” The old worker stopped at 5.577 seconds.

The worker now allows up to 375 codec frames (30 seconds), disables the relative
text-token cap, and independently refuses more than 720,000 emitted 24 kHz PCM
samples. Natural EOS is still required. A generation-limit failure exits without
a completion event; callers must not automatically replay partially spoken text.

Qualification on Dax's GTX1080:

- Container build and 11 worker protocol tests pass, including an engine double
  that exceeds the PCM budget; no excess samples or false completion escape.
- Fresh and post-cancellation fixture complete at natural EOS, 5.977 seconds;
  an ASR-assisted public-audio screen recognized all expected words.
- Cancellation settled in 322 ms in that direct worker trial.
- A 53-word multi-sentence diagnostic still reached the 30-second limit without
  completing; the failure remained explicit.

The larger generation/cache allocation also changed a different two-sentence
fixture from 5.497 seconds to 19.097 seconds, including a 9.3-second near-silent
interval. This is an observed behavior change, not a diagnosed kernel bug or a
perceptual acceptance claim. Do not deploy this budget change alone as a complete
speech-quality repair. Nine single-sentence controls completed in 2.38–4.22 seconds,
with no quiet run longer than 0.44 seconds. The separate supervisor's optional
sentence segmentation is being qualified while retaining one outer cancellable
request and its whole-request audio limit.

Candidate worker SHA256: `0fca8e19d60a379beb6ef20063efb2dfa63bd3f8c7f30b5279fbc14c95598dea`.
Build directory: `~/voice-services/qualification/fork-build-30s-budget-v1` on Dax.
Evidence under the host setup repository: `hosts/dax/qualification/results/`
directories `tts-short-budget-diagnosis-v1`, `tts-30s-budget-v1`,
`tts-sentence-budget-v1`, and `tts-retention-sentences-gpu-v2`.
No user recordings, model weights, or credentials are committed here.
