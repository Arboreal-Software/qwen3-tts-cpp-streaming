# Dax experimental qualification

Baseline: upstream 74422bab4998ad69e0a67ba105654dadbeec9b3a. GGML is vendored,
not a submodule; its baseline tree is in provenance.json. Upstream and dependency
license notices remain intact. These changes are **not production-qualified**.

Dax currently has one GTX1080 and an i7-3770K. The second GPU and RX6800XT remain
pending. Engine patches were copied unchanged from experiments; the hardcoded CPU
target is now a parameter and diagnostic targets use repo-relative paths.
No running experiment or Defiant service was migrated by creating this fork.

## Reproduce a container build

From this checkout, create a separate build directory, then run:

```sh
mkdir -p build-dax
docker run --rm --device nvidia.com/gpu=all \
  -v "$PWD:/src:ro" -v "$PWD/build-dax:/build" \
  nvidia/cuda@sha256:d49bb8a4ff97fb5fe477947a3f02aa8c0a53eae77e11f00ec28618a0bcaa2ad1 \
  bash /src/qualification/dax/build.sh
```

Packages install inside the container. Use targeted CDI UUIDs on shared hosts.
The kernel test compares 80 shapes against original traversal and a double
precision CPU reference. It is a copied standalone fixture with a source drift
guard, not an invocation of GGML's production scheduler.

## Model-dependent checks

Mount pinned GGUFs separately; hashes are in TTS-MODEL-RECEIPT.md.
Run inside the same GPU container with model/build mounts and writable /results:

```sh
DAX_PROFILE_DECODER_OPS=1 DAX_CUDA_TRANSPOSE_F32=1 \
  /build/dax_decoder_stages /models/qwen3-tts-tokenizer-f16.gguf
DAX_CUDA_TRANSPOSE_F32=1 DAX_PROBE_OUTPUT=/results \
  /build/dax_persistent_probe /models /src/reference/priestley_0.6b_f16.json
```

The reuse probe currently exits 4 when repeat waveforms differ after an intervening
request. This diagnostic failure also occurs without cancellation; do not hide it
or equate it to proven cross-session leakage. Compare cancellation against
DAX_COMPLETE_INTERMEDIATE=1. Do not share a mutable engine across users.

DAX_CUDA_TRANSPOSE_F32 and DAX_PROFILE_DECODER_OPS use **presence** semantics:
even setting them to 0 enables them; unset to disable. The first casts transpose
weights to F32 for CUDA dispatch. The second synchronizes operator evaluation and
changes timing; use it for placement, not throughput. Detailed generation timing
is a separate QWEN3_TTS_TIMING CMake option.

## Runtime blocker

A subsequent instrumented candidate run aborted during streaming with CUDA illegal
memory access. See [failure evidence](TTS-CUDA-FAILURE.md). Root cause is unknown;
prior successful benchmarks do not establish stability. Do not deploy this candidate.

## Evidence and remaining gates

Reports are snapshots from /home/claire/inference-setup/hosts/dax/qualification/;
relative results/ links refer there. Raw logs, WAVs, archives and models are not
published. Fresh fork build integration passed on Dax; see FORK-BUILD-RESULT.md. Runtime qualification remains incomplete.

One paired test improved 11.497s audio synthesis from 12.131s to 7.562s,
first PCM 814ms to 336ms. Two overlapping requests took about 14.4s each and
failed to sustain real time. Unpaced control confirmed pacing is not the bottleneck.
A four-CPU quota also left the candidate near 7.5s solo /14.4s overlap.
Detailed generation profiling and paced CPU ASR testing remain active on Dax.

Still required: representative listening/intelligibility, memory stability,
interruption before first PCM, cancellation/reuse isolation, sustained simultaneous
streams, ASR coexistence and browser/headset acceptance. Numerical waveform
closeness is not a listening result.

Baseline rollback: upstream baseline plus only the Linux compatibility fix, with
the same pinned models/image/CPU flags. Never deploy this development branch implicitly.
