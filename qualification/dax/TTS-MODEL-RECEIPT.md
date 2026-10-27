# Dax TTS model preparation

2026-10-27: download and conversion containers both exited0. No GPU synthesis yet.

Official sources:

- Qwen/Qwen3-TTS-12Hz-0.6B-Base, revision `5d83992436eae1d760afd27aff78a71d676296fc`.
- Qwen/Qwen3-TTS-Tokenizer-12Hz, revision `7dd38ad4e9bad454aae9cd937d0cd577604fe229`.

F16 conversion by pinned maxhorizons C++ engine conversion scripts at `74422bab4998ad69e0a67ba105654dadbeec9b3a`, executed in the previously qualified CPU PyTorch container. Conversion selected the tokenizer embedded in the Base checkpoint; the separately downloaded official tokenizer is retained for comparison/provenance.

Outputs in Dax `~/voice-services/qualification/models`:

| File | SHA256 |
| --- | --- |
| qwen3-tts-0.6b-f16.gguf | f9d599e538d0400ec5b61c9bc342dadaab7ec8dae0455281350641be57a02f02 |
| qwen3-tts-tokenizer-f16.gguf | d1ad9660bd99343f4851d5a4b17e31f65648feb3559f6ea062ae6575e5cd9d90 |

Remote evidence includes model download-lock/download-receipt JSON files, tts-conversion-install-report.json, tts-conversion-packages.txt, qwen-tts-convert.log and tts-gguf-sha256.txt. Successful conversion does not prove fidelity, speech quality or runtime compatibility; those are separate model-backed checks.
