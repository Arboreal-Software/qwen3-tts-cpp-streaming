# Frozen evidence for GPT-6 Pro CUDA investigation

Owner-requested follow-up to the completed Extra High review. No live-service
changes are authorized by this packet. Only public fixture text and technical
diagnostics are included; no credentials, recordings or private transcripts.

## Source identity

Original experimental source HEAD:74422bab4998ad69e0a67ba105654dadbeec9b3a.
Apply [original-experiment.patch](original-experiment.patch) to reconstruct tracked
original runtime changes. Current retained git diff SHA256 matches the locally
collected pre-fork patch:efeec5b966c3b1ea4150c9ff5e0f54f2735f96501a5b06f22adb8d86af1ff65b.
Diagnostic helper source paths in the patch were outside that tree; public later
fork equivalents live under qualification/dax. They were not the CLI failure path.
Vendored GGML tree:b1e00d0467ff3e8a7ade3bc6031b756b4cfe0c13.

Review current runtime repairs at b8875c89ba287721bb82405eda86830388fca21e:
4f72daa adds scope cleanup of borrowed timing state; b8875c8 adds opt-in separate
component backends and a lifecycle lock for the legacy shared backend. Isolation
requires literal ARBOREAL_ISOLATED_BACKENDS=1. Source at9a65740244a68523c1cafc834ac9538ec44c2a93
is identical runtime plus initial result documentation. These are experimental,
not deployed. The original failing build is distinct from the later fork build.

## Failure and retained-build provenance

The original failure had timing ON, F32 transpose enabled, async decode, offgrid
callback profile, unpaced delivery, seed42, fixed public fixture sentence. Original
container CPU quota4; memory6GiB. Later40-run stress used CPU quota2, same retained
binary. This is a potentially meaningful scheduling difference. Full invocation
appears at the end of the failure excerpt below. DAX_PROFILE_DECODER_OPS was not
specified by the failure launcher. F32 flags use presence semantics in original.

A contemporaneous binary SHA at the instant of the crash was not recorded. The
retained original build at its original path has the SHA below, matching the
later stress-run hash. Symbolization below uses that retained executable and
libraries, NOT the new fork build. Release symbols resolve functions but mostly
not line numbers; do not claim stronger identity or debug precision than this.

```
Original retained source HEAD:
74422bab4998ad69e0a67ba105654dadbeec9b3a
Original retained source patch SHA:
efeec5b966c3b1ea4150c9ff5e0f54f2735f96501a5b06f22adb8d86af1ff65b  -
Driver:
name, driver_version
NVIDIA GeForce GTX 1080, 580.178.04
Failure container selected configuration:
nvidia/cuda@sha256:d49bb8a4ff97fb5fe477947a3f02aa8c0a53eae77e11f00ec28618a0bcaa2ad1 ["bash","/work/run-generation-profile.sh"] cpus=4000000000 memory=6442450944 exit=134
Recorded CMake settings:
See separately recovered CMake flags below.
0386bb3206d48285b3d4c2565778aaa1606ddd088dc736d07e002cb2c0739a98  /build/apps/streaming_cli/qwen3_streaming_cli
8c7b33314cbf6645df4c8ada1aaea85e40958c762ac9f0e5e079ed112736aa1c  /build/engine/ggml/src/ggml-cuda/libggml-cuda.so.0
a7180674d4f476cb6b60c8c77755e2980586ddd262c193e5d969cec76bc4cfb4  /build/engine/ggml/src/libggml-base.so.0
gcc (Ubuntu 11.4.0-1ubuntu1~22.04) 11.4.0
Cuda compilation tools, release 12.6, V12.6.85
Build cuda_12.6.r12.6/compiler.35059454_0
EXECUTABLE_OFFSETS
qwen3_tts::TTSTransformer::forward_step(float const*, int, std::vector<float, std::allocator<float> >&, std::vector<float, std::allocator<float> >*)
??:?
qwen3_tts::TTSTransformer::generate_streaming(int const*, int, float const*, int, std::vector<int, std::allocator<int> >&, std::function<bool (std::vector<int, std::allocator<int> > const&, int, bool)>, int, float, float, int, float, float, int const*, int, qwen3_tts::tts_generation_first_frame_profile*)
??:?
qwen3_tts::pipeline_internal::ops::synthesize_internal(qwen3_tts::Qwen3TTS&, std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const&, float const*, qwen3_tts::tts_params const&, qwen3_tts::tts_result&)
??:?
qwen3_tts::Qwen3TTS::synthesize_with_speaker_embedding(std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const&, std::vector<float, std::allocator<float> > const&, qwen3_tts::tts_params const&)
??:?
Qwen3StreamingTts::synthesize_streaming(std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const&, TtsStreamOptions const&, std::function<bool (TtsStreamChunk const&)>)
??:?
main
??:?
qwen3_tts::AudioTokenizerDecoder::decode_batch(int const*, int, int, std::vector<std::vector<float, std::allocator<float> >, std::allocator<std::vector<float, std::allocator<float> > > >&)
??:?
qwen3_tts::AudioTokenizerDecoder::decode(int const*, int, std::vector<float, std::allocator<float> >&)
??:?
qwen3_tts::pipeline_internal::ops::synthesize_internal(qwen3_tts::Qwen3TTS&, std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const&, float const*, qwen3_tts::tts_params const&, qwen3_tts::tts_result&)::{lambda(qwen3_tts::pipeline_internal::ops::synthesize_internal(qwen3_tts::Qwen3TTS&, std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const&, float const*, qwen3_tts::tts_params const&, qwen3_tts::tts_result&)::DecodeJob const&)#4}::operator()(qwen3_tts::pipeline_internal::ops::synthesize_internal(qwen3_tts::Qwen3TTS&, std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const&, float const*, qwen3_tts::tts_params const&, qwen3_tts::tts_result&)::DecodeJob const&) const
pipeline_synthesize.cpp:?
std::thread::_State_impl<std::thread::_Invoker<std::tuple<qwen3_tts::pipeline_internal::ops::synthesize_internal(qwen3_tts::Qwen3TTS&, std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const&, float const*, qwen3_tts::tts_params const&, qwen3_tts::tts_result&)::{lambda()#10}> > >::_M_run()
pipeline_synthesize.cpp:?
CUDA_LIBRARY_OFFSETS
ggml_cuda_error(char const*, char const*, char const*, int, char const*)
??:?
ggml_backend_cuda_synchronize(ggml_backend*)
tmpxft_0000054b_00000000-6_ggml-cuda.cudafe1.cpp:?

CMAKE_BUILD_TYPE:STRING=Release
CMAKE_CUDA_ARCHITECTURES:UNINITIALIZED=61
CMAKE_CUDA_COMPILER:FILEPATH=/usr/local/cuda/bin/nvcc
CMAKE_CXX_COMPILER:FILEPATH=/usr/bin/c++
GGML_AVX:BOOL=ON
GGML_AVX2:BOOL=OFF
GGML_BMI2:BOOL=OFF
GGML_CUDA_GRAPHS:BOOL=OFF
GGML_F16C:BOOL=ON
GGML_FMA:BOOL=OFF
GGML_NATIVE:BOOL=OFF
QWEN3_TTS_TIMING:BOOL=ON
```

## Relevant full failure tail: both backtraces

Full locally retained generation-profile.log SHA256:
8669fc36247c91a6470cda5c449bffbcde735ea03c3759005d66d5f700de8a44.
Earlier portions are package installation/build output and preceding PCM windows.
The full failure tail follows without removing either thread's stack.

```text
[stream] window_available window_index=8 samples=19200 audio_ms=800.0 wall_ms_since_request=4068 wall_ms_since_prev_window=512 cumulative_audio_ms=6216.9 player_queued_audio_ms=0.0
       9        74        78        88        14        105        7125       19200  ok
[stream] window_available window_index=9 samples=19200 audio_ms=800.0 wall_ms_since_request=4584 wall_ms_since_prev_window=516 cumulative_audio_ms=7016.9 player_queued_audio_ms=0.0
CUDA error: an illegal memory access was encountered
  current device: 0, in function ggml_backend_cuda_synchronize at /work/qwen-tts-conv-source/engine/ggml/src/ggml-cuda/ggml-cuda.cu:2948
  cudaStreamSynchronize(cuda_ctx->stream())
/work/qwen-tts-conv-source/engine/ggml/src/ggml-cuda/ggml-cuda.cu:98: CUDA error
CUDA error: an illegal memory access was encountered
  current device: 0, in function ggml_backend_cuda_synchronize at /work/qwen-tts-conv-source/engine/ggml/src/ggml-cuda/ggml-cuda.cu:2948
  cudaStreamSynchronize(cuda_ctx->stream())
/work/qwen-tts-conv-source/engine/ggml/src/ggml-cuda/ggml-cuda.cu:98: CUDA error
/work/qwen-tts-conv-build/engine/ggml/src/libggml-base.so.0(+0x1883b)[0x7260fe00683b]
/work/qwen-tts-conv-build/engine/ggml/src/libggml-base.so.0(ggml_print_backtrace+0x21f)[0x7260fe006c9f]
/work/qwen-tts-conv-build/engine/ggml/src/libggml-base.so.0(ggml_abort+0x152)[0x7260fe006e72]
/work/qwen-tts-conv-build/engine/ggml/src/ggml-cuda/libggml-cuda.so.0(+0x189fe6)[0x7260f8989fe6]
/work/qwen-tts-conv-build/engine/ggml/src/ggml-cuda/libggml-cuda.so.0(+0x18b19f)[0x7260f898b19f]
/work/qwen-tts-conv-build/engine/ggml/src/libggml-base.so.0(ggml_backend_sched_synchronize+0x2e)[0x7260fe0200ee]
/work/qwen-tts-conv-build/engine/ggml/src/libggml-base.so.0(ggml_backend_sched_graph_compute+0x1e)[0x7260fe0247de]
/work/qwen-tts-conv-build/apps/streaming_cli/qwen3_streaming_cli(+0x883c1)[0x586dc9c413c1]
/work/qwen-tts-conv-build/apps/streaming_cli/qwen3_streaming_cli(+0x80eee)[0x586dc9c39eee]
/work/qwen-tts-conv-build/apps/streaming_cli/qwen3_streaming_cli(+0x3442e)[0x586dc9bed42e]
/work/qwen-tts-conv-build/apps/streaming_cli/qwen3_streaming_cli(+0x38bb5)[0x586dc9bf1bb5]
/work/qwen-tts-conv-build/apps/streaming_cli/qwen3_streaming_cli(+0x23cc3)[0x586dc9bdccc3]
/work/qwen-tts-conv-build/apps/streaming_cli/qwen3_streaming_cli(+0x15d79)[0x586dc9bced79]
/lib/x86_64-linux-gnu/libc.so.6(+0x29d90)[0x7260f82edd90]
/lib/x86_64-linux-gnu/libc.so.6(__libc_start_main+0x80)[0x7260f82ede40]
/work/qwen-tts-conv-build/apps/streaming_cli/qwen3_streaming_cli(+0x1bf55)[0x586dc9bd4f55]
/work/qwen-tts-conv-build/engine/ggml/src/libggml-base.so.0(+0x1883b)[0x7260fe00683b]
/work/qwen-tts-conv-build/engine/ggml/src/libggml-base.so.0(ggml_print_backtrace+0x21f)[0x7260fe006c9f]
/work/qwen-tts-conv-build/engine/ggml/src/libggml-base.so.0(ggml_abort+0x152)[0x7260fe006e72]
/work/qwen-tts-conv-build/engine/ggml/src/ggml-cuda/libggml-cuda.so.0(+0x189fe6)[0x7260f8989fe6]
/work/qwen-tts-conv-build/engine/ggml/src/ggml-cuda/libggml-cuda.so.0(+0x18b19f)[0x7260f898b19f]
/work/qwen-tts-conv-build/engine/ggml/src/libggml-base.so.0(ggml_backend_sched_synchronize+0x2e)[0x7260fe0200ee]
/work/qwen-tts-conv-build/engine/ggml/src/libggml-base.so.0(ggml_backend_sched_graph_compute+0x1e)[0x7260fe0247de]
/work/qwen-tts-conv-build/apps/streaming_cli/qwen3_streaming_cli(+0x94dab)[0x586dc9c4ddab]
/work/qwen-tts-conv-build/apps/streaming_cli/qwen3_streaming_cli(+0x94eee)[0x586dc9c4deee]
/work/qwen-tts-conv-build/apps/streaming_cli/qwen3_streaming_cli(+0x2fb93)[0x586dc9be8b93]
/work/qwen-tts-conv-build/apps/streaming_cli/qwen3_streaming_cli(+0x30aed)[0x586dc9be9aed]
/lib/x86_64-linux-gnu/libstdc++.so.6(+0xdc253)[0x7260f86b0253]
/lib/x86_64-linux-gnu/libc.so.6(+0x94ac3)[0x7260f8358ac3]
/lib/x86_64-linux-gnu/libc.so.6(clone+0x44)[0x7260f83e9a04]
timeout: the monitored command dumped core
/work/run-generation-profile.sh: line 12:  3684 Aborted                 DAX_CUDA_TRANSPOSE_F32=1 timeout 180 /work/qwen-tts-conv-build/apps/streaming_cli/qwen3_streaming_cli -m /work/models --model-identifier qwen3-tts-0.6b-f16 --speaker-embedding /work/qwen-tts-source/reference/priestley_0.6b_f16.json --tts-profile offgrid-callback --no-paced-audio-delivery --seed 42 -t 'Dax is ready for a voice test. This sentence checks clear speech and timing. We will keep the original conversation safe while testing whether two people can receive spoken replies at the same time.' -o /work/generation-profile.wav --no-play-streaming --simulate-stream-callback
```

## Completed review, independently checked findings and current experiments

Prior review: Slack thread1793081959.994279, result1793083089.870629 and
1793083089.907329. It identified upstream process-global shared ggml_backend_t,
mutable CUDA streams/handles/pools used by overlapping transformer and decoder
threads, plus unlocked lifecycle. Local source confirms backend sharing; it is
NOT proven as the illegal-access root cause. Review found bounded transpose
indexing valid for supported contiguous3D layouts; huge-shape integer narrowing
is a pre-existing risk, unlikely in this fixture. It considered sequential
scheduler reset/readback and joined worker lifetimes sound, with shared backend
concurrency the leading hypothesis. It found the timing pointer dangling on early
returns, now scope-cleaned in4f72daa. Decoder profiler stack pointer unregisters
on normal/returned error paths; RAII on thrown exceptions remains worth checking.

- Six fresh processes passed:3 CUDA_LAUNCH_BLOCKING=1 and3=0. Concurrent CPU
  compilation occurred; timings not performance evidence.
- compute-sanitizer memcheck: exit0, ERROR SUMMARY:0, full11.497s output;
  instrumented477.261s is not performance evidence.
- Original retained timing-enabled binary:40 fresh-process tests passed (20async,
  20sync alternating). Median7.535s/7.554s for11.497s audio; firstPCM338/327ms.
  CPU quota2 versus original failure4; no root-cause claim.
- Later fork candidate with default timing configuration:12 tests (3repeats of
  isolation OFF/ON crossed with async/sync) pass. All12WAV SHA256 values identical.
  Sharedasync7.512s,sharedsync7.541s,separateasync7.465s,separatesync7.544s medians;
  firstPCM327–338ms, GPUpeak2447MiB shared vs2449MiB separate (100ms samples).
  Timing-enabled repair not yet rebuilt/tested. One earlier launcher exit127 was
  missing /build library mount before synthesis; excluded, not a CUDA failure.
- Combined2ASRstreams+1separate-backend TTS worker: all3TTS invocations pass,
  8.289–8.335s for11.497s audio, firstPCM338–392ms; sampledGPUpeak5839MiB.
  ASR final transcripts match references, maxlag264ms; no end-to-end deployment.
- Persistent separate-backend probe: two initial repeated prompts exactly match;
  after a different prompt, the original prompt's last49920samples differ
  (same158805total; maxabs0.824378,RMS0.055358). IDENTICAL difference metrics with
  intervening request cancelled or completed. Both probes exit4 (exact-repeat
  mismatch), not success. Cancellation27.813ms, zero later callbacks; next request
  completes. Shared backend had the same history-dependent symptom earlier.
  This could be a separate issue. No intelligibility or code-sequence localization
  done; do not equate waveform variation with semantic leakage.

## Requested investigation delta

Do not simply repeat the prior review. Trace exact buffer ownership and stream
ordering across original patch and current separate-backend changes. Assess which
shared CUDA state remains process/global after per-component backend allocation,
whether separate contexts truly eliminate the identified hazard, and whether
other lifetime/cast/layout/kernel/state issues explain the fault or waveform
symptom. Separate proven defects from hypotheses. Rank by exact file/function/line,
mechanism, supporting/conflicting evidence, confidence, smallest discriminating
test. Propose bounded instrumentation/patches only where justified; include how to
localize the FIRST bad launch rather than the eventual synchronize. Compare timing
ON/OFF,4CPU/2CPU,async/sync and cast presence where informative. Assess shutdown/
poisoned-context error propagation: no auto-recovery service is yet qualified.

No hardware execution or mutations, no recursive research fan-out. Report actual
executing model/route and substitutions. Review asynchronously; local work continues.
