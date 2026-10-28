#!/usr/bin/env bash
# Inside pinned CUDA devel container: repo /src read-only; build /build writable.
set -euo pipefail
apt-get update
apt-get install -y --no-install-recommends ca-certificates cmake ninja-build g++ python3
cmake -S /src -B /build -G Ninja -DCMAKE_BUILD_TYPE=Release \
 -DQWEN3_TTS_CPU_ARCH=ivybridge -DQWEN3_TTS_COREML=OFF \
 -DQWEN3_TTS_CUDA=ON -DCMAKE_CUDA_ARCHITECTURES=61 \
 -DGGML_NATIVE=OFF -DGGML_AVX=ON -DGGML_AVX2=OFF \
 -DGGML_FMA=OFF -DGGML_F16C=ON -DGGML_BMI2=OFF -DGGML_CUDA_GRAPHS=OFF \
 -DARBOREAL_BUILD_QUALIFICATION=ON
cmake --build /build --parallel 2
ctest --test-dir /build --output-on-failure -R '^wrapper_contract$'
python3 /src/qualification/dax/check-kernel-source.py
nvcc -O3 -arch=sm_61 /src/qualification/dax/conv-equivalence.cu -o /build/conv-equivalence
/build/conv-equivalence
