# Timing-enabled profiler repair — initial Dax GPU result

Source0f099f70663f8d067084d2b36311c03b02466ca1, including scoped callback
repair bcb980b. Release IvyBridge/SM61, timing ON, graphs OFF, no AVX2/FMA/BMI2.
Exact CLI SHA256:
0badfe7a339f73e050119bc18ce8bb2dbd64096e13b90d9c635b2c4f2e67e5dd.

Full containerized build passed, decoder_callback_scope CTest passed, and CUDA
source matched the existing standalone kernel fixture. CPU-only compilation
required explicit CUDA stub link plus transitive libcuda.so.1 link-time lookup;
no driver stub was installed on the host or runtime. Two failed build logs were
preserved before successful attempt3. Runtime used the actual NVIDIA driver.

GTX1080, CPU4/6GB, literal ARBOREAL_ISOLATED_BACKENDS=1, F32 transpose and
DAX_PROFILE_DECODER_OPS enabled. Evaluation ASR stayed loaded, with zero clients
at admission. One fresh process per mode; same public fixture and seed42.

| Mode | Synthesis wall | Audio duration | Operation-profile records | Exit |
| --- | ---: | ---: | ---: | ---: |
| Async | 7.177s | 11.4969s | 16014 | 0 |
| Sync | 7.462s | 11.4969s | 16014 | 0 |

Wrapper contract runtime test also exited0. WAV hashes matched exactly:
ffa3d8a67e77d7d9e32de2136025555f16e41bc0d54488f5fa7504e41b83b1fb.

This qualifies initial timing-enabled repair execution, not original-crash root
cause, general stability, persistent-worker reuse, two simultaneous TTS users,
CUDA recovery, or uncontended performance. Profiling changes scheduling and ASR
was resident. Persistent waveform drift remains open. No live voice cutover.
Local raw evidence: inference-setup/hosts/dax/qualification/results/
profiler-cleanup-gpu-20261027T075332Z and profiler-cleanup-proof.tgz.
