# Long-text completion diagnostic — 2026-10-27

A public synthetic53-word passage omitted its final20 words even though the
persistent worker emitted D. Repeated recognition of the full saved PCM and
fresh streams containing only its tail reproduced the omission. PCM contained
roughly ten seconds of near-silence before speech resumed. These observations
are not attributed to the owner's historical drops.

An isolated diagnostic candidate changes only the worker's opt-in print_timing
setting. It does not change sampling, scheduling or wire frames. With
`ARBOREAL_TTS_DIAGNOSTICS=1`, bounded supervisor logs include the engine's existing
EOS/token-limit reason. Default behavior remains unchanged. Diagnostics must not
be written into the binary event pipe.

Built on Dax from a7224c5 plus the diagnostic patch, using the existing pinned
CUDA12.6/Ivy Bridge/SM61 build. Nine actual-worker protocol tests with an engine
double passed. The GPU run used the existing0.6B F16 model, Lana embedding,
seed42 and default sampling. It did not restart either speech service. Source,
build and result directories are separate from the deployed ones.

| Input | Engine termination | Audio duration | Generation wall time |
| --- | --- | --- | --- |
| Entire six-sentence passage | safety_limit,300 audio tokens |23.977s|14.915s|
| Sentence1 | eos,37 |2.937s|1.901s|
| Sentence2 | eos,34 |2.697s|1.761s|
| Sentence3 | eos,33 |2.617s|1.714s|
| Sentence4 | eos,33 |2.617s|1.714s|
| Sentence5 | eos,50 |3.977s|2.563s|
| Sentence6 | eos,50 |3.977s|2.565s|

First audio was335ms for the passage and321–322ms for individual sentences.
ASR-assisted screening matched all words in all six sentence-sized outputs;
the diagnostic long passage reproduced the same20-word omission. Punctuation
was normalized; this is not independent human perceptual acceptance.

The engine's text-relative ceiling is5audio frames per text token, with minimum64
and absolute4096. Its tts_result records EOS and limit flags, but the streaming
wrapper returns success when synthesis otherwise succeeded, and the persistent
worker emits D if any audio was emitted. Therefore D currently means successful
bounded generation, not complete spoken coverage. Do not infer content fidelity
from frame-count continuity, HTTP success or D alone.

The existing voice host already splits speech at sentence/clause boundaries with
40-word/260-character bounds (`native-voice/phrase-buffer.mjs`). This unusually
long multi-sentence request is NOT proof that its normal path sends equivalent
requests or that more splitting is needed there. Before changing live behavior,
test the actual emitted phrase sizes, expose truncation as distinct from natural
completion, and measure content/interruptions on that path. Raising the safety
limit alone would not explain or repair the long silence and is not deployed.

Evidence in inference-setup:

- `hosts/dax/qualification/results/tts-content-termination-v1/tts-content-termination-v1/`
- `experiments/qev-voice/results/dax-tts-sentence-content-20261027T1344/`
- Original `experiments/qev-voice/results/dax-tts-recovery-20261027T1316/`
- Tail isolation `experiments/qev-voice/results/dax-tts-tail-20261027T1333/`
- Runners `hosts/dax/qualification/{build-tts-termination-diagnostics.sh,retry-tts-termination-build.sh,run-tts-content-termination.sh,tts-content-termination-probe.py}`

The first build retry addressed only an inherited temporary CUDA stub symlink;
its failure log is preserved. No model/session/accepted-input replay or live
deployment occurred. CUDA fault recovery and cross-user isolation remain separate.
