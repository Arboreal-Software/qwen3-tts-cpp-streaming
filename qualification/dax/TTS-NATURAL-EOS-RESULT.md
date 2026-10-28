# Refuse truncated worker completion — 2026-10-27

The persistent worker now requires the underlying generation to reach natural
EOS before reporting D. The streaming wrapper exposes this as an opt-in option;
other callers retain their previous default. Engine safety limits are unchanged.
This prevents a completed transport from falsely claiming naturally finished
generation; EOS alone still does not prove spoken-content fidelity.

If the limit is reached after partial audio, the wrapper returns failure and the
worker exits through its existing failure path. There is no completion event,
automatic replay or silent retry. Consumers must preserve the text conversation
and treat already emitted speech as partial. This implementation uses the existing
failure/reconnect protocol; structured recoverable truncation metadata is future
work, not claimed here. Cancellation still emits C and allows intentional reuse.

Qualification on Dax's GTX1080 with the same0.6B F16/Lana/seed42 settings:

- Ten actual-worker protocol tests using the engine double passed, including
  partial audio followed by limit failure with no D.
- Real long passage reproduced the300-token safety limit, emitted23.977seconds
  of partial audio, then exited70 with EOF and no D.
- A separately constructed worker completed a new short request naturally.
- Cancellation settled in323.3ms with C; a subsequent short request on that same
  worker completed with D. The replacement worker exited0 after input close.
- No live service was stopped or replaced by these tests.

Candidate worker SHA256:
`b49af3c65fe4e153fdfb18fced0df0bef43dd6a049da1030cd97fb06cc40f3fb`.
Patch over the diagnostic parent:
`295d84911fe73bce16cba2526e2da0e30622dba037c4f83ca85e08ae0ddda548`.
Raw receipts: inference-setup
`hosts/dax/qualification/results/tts-natural-eos-v1/tts-natural-eos-v1/`.
Reproduction scripts: `build-tts-natural-eos.sh`, `tts-natural-eos-probe.py`,
and `run-tts-content-termination.sh` in that workspace's qualification directory.

Additional scope check before this repair: seven phrases emitted by the actual
host PhraseBuffer, up to40words/229characters, all reached natural EOS. Six matched
every normalized word in ASR; the longest differed by a single omitted “and”.
That difference is unassigned between synthesis and recognition pending listening.
These are development fixtures, not general fidelity or cross-user acceptance.
The live host already splits at sentence/clause boundaries; no second splitter
was introduced. CUDA fault recovery remains separately qualified work.
