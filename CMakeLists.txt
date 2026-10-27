cmake_minimum_required(VERSION 3.20)
project(qwen3_tts_workspace LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(CTest)

add_subdirectory(engine)

option(QWEN3_TTS_BUILD_STREAMING_WRAPPER "Build the C++ streaming integration wrapper" ON)
option(QWEN3_TTS_BUILD_STREAMING_CLI "Build the streaming command-line harness" ON)
if(QWEN3_TTS_BUILD_STREAMING_WRAPPER)
    add_subdirectory(apps/streaming_cli)
endif()

if(BUILD_TESTING AND QWEN3_TTS_BUILD_STREAMING_WRAPPER)
    add_subdirectory(tests)
endif()

# Qualification executables are opt-in, not normal service builds.
option(ARBOREAL_BUILD_QUALIFICATION "Build Dax diagnostic executables" OFF)
if(ARBOREAL_BUILD_QUALIFICATION)
    add_executable(dax_persistent_probe qualification/dax/tts-persistent-probe.cpp)
    target_link_libraries(dax_persistent_probe PRIVATE qwen3_tts_streaming)
    add_executable(dax_decoder_stages qualification/dax/decoder-stages.cpp)
    target_link_libraries(dax_decoder_stages PRIVATE audio_tokenizer_decoder)
endif()
