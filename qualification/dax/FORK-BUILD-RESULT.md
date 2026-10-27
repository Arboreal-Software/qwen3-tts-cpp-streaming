# Fresh fork build — 2026-10-27

Exact tested commit: 7a686960dcd26e573f7b16bc21e959aca2883ebd.
A separate checkout/build on Dax used qualification/dax/build.sh and the pinned
CUDA12.6 container with SM61 and Ivy Bridge flags. All220 build steps completed.
Wrapper contract1/1 passed. CUDA source guard passed; standalone80-shape,
309792-sample original/candidate equality and double-precision reference check passed.

Raw log: inference-setup/hosts/dax/qualification/results/fork-qualification.log
SHA256 a39be69ca2380ffcbb04861751598e74048f9151f99c0a751e9ec3991dd54000.

This establishes fresh build/test integration of the migrated commits. It does
not establish stable model-serving behavior, perceptual quality or two-user
capacity. TTS-CUDA-FAILURE.md remains an open deployment blocker; compute-sanitizer
is still running against the original instrumented candidate. Subsequent doc-only
commits do not change the runtime bytes tested here.
