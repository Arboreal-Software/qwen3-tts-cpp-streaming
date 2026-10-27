add_executable(qwen3_decoder_callback_scope_test decoder_callback_scope_test.cpp)
target_compile_features(qwen3_decoder_callback_scope_test PRIVATE cxx_std_17)
target_include_directories(qwen3_decoder_callback_scope_test PRIVATE
    ${CMAKE_SOURCE_DIR}/engine/src ${CMAKE_SOURCE_DIR}/engine/ggml/include)
add_test(NAME decoder_callback_scope COMMAND qwen3_decoder_callback_scope_test)
set_tests_properties(decoder_callback_scope PROPERTIES LABELS "fast;contract")

add_executable(qwen3_wrapper_contract_test
    wrapper_contract_test.cpp
)
target_link_libraries(qwen3_wrapper_contract_test PRIVATE qwen3_tts_streaming)

add_test(NAME wrapper_contract COMMAND qwen3_wrapper_contract_test)
set_tests_properties(wrapper_contract PROPERTIES LABELS "fast;contract")

add_executable(qwen3_wrapper_model_test
    wrapper_model_test.cpp
)
target_link_libraries(qwen3_wrapper_model_test PRIVATE qwen3_tts_streaming)

add_executable(qwen3_decoder_batch_test
    decoder_batch_test.cpp
)
target_link_libraries(qwen3_decoder_batch_test PRIVATE audio_tokenizer_decoder)

if(WIN32)
    foreach(test_target qwen3_wrapper_contract_test qwen3_wrapper_model_test qwen3_decoder_batch_test)
        add_custom_command(TARGET ${test_target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${CMAKE_BINARY_DIR}/bin
                $<TARGET_FILE_DIR:${test_target}>
        )
    endforeach()
endif()

option(QWEN3_TTS_ENABLE_MODEL_TESTS "Register model-backed tests with CTest" OFF)
if(QWEN3_TTS_ENABLE_MODEL_TESTS)
    add_test(
        NAME wrapper_model_contract
        COMMAND qwen3_wrapper_model_test
            --models ${CMAKE_SOURCE_DIR}/models
            --model qwen3-tts-0.6b-f16
            --embedding ${CMAKE_SOURCE_DIR}/reference/priestley_0.6b_f16.json
            --output-dir ${CMAKE_BINARY_DIR}/acceptance
    )
    set_tests_properties(wrapper_model_contract PROPERTIES
        LABELS "model;gpu;contract"
        TIMEOUT 120
    )
    add_test(
        NAME decoder_physical_batch
        COMMAND qwen3_decoder_batch_test
            ${CMAKE_SOURCE_DIR}/models/qwen3-tts-tokenizer-f16.gguf
            45
    )
    set_tests_properties(decoder_physical_batch PROPERTIES
        LABELS "model;gpu;batch"
        TIMEOUT 120
    )
endif()

add_executable(qwen3_sampling_safety_test sampling_safety_test.cpp)
target_include_directories(qwen3_sampling_safety_test PRIVATE ${CMAKE_SOURCE_DIR}/engine/src)
add_test(NAME sampling_safety COMMAND qwen3_sampling_safety_test)
set_tests_properties(sampling_safety PROPERTIES LABELS "fast;contract")

add_executable(qwen3_kv_active_extent_test kv_active_extent_test.cpp)
target_link_libraries(qwen3_kv_active_extent_test PRIVATE ggml ggml-cpu)
add_test(NAME kv_active_extent COMMAND qwen3_kv_active_extent_test)
set_tests_properties(kv_active_extent PROPERTIES LABELS "fast;contract")

if(QWEN3_TTS_CUDA)
    target_link_libraries(qwen3_kv_active_extent_test PRIVATE ggml-cuda)
    target_compile_definitions(qwen3_kv_active_extent_test PRIVATE ARBOREAL_TEST_CUDA=1)
endif()
