# Worker speech duration budget

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
