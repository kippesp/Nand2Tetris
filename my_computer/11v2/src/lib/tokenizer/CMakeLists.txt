set(LIBRARY_TARGET_NAME tokenizer)

set(${LIBRARY_TARGET_NAME}_SRCS
    jack_tokenizer.cpp
    jack_token.cpp

    jack_tokenizer.h
    jack_token.h
)

add_library(${LIBRARY_TARGET_NAME} ${${LIBRARY_TARGET_NAME}_SRCS})

target_include_directories(${LIBRARY_TARGET_NAME} PRIVATE ..)

# Add turnt tests for tokenizer phase

set(ENV{CTEST_OUTPUT_ON_FAILURE} 1)
file(GLOB TOKENIZER_TURNT_TESTS ${CMAKE_SOURCE_DIR}/src/turnt_tests/tokenizer/*.jack)
foreach(TOKENIZER_TURNT_TEST ${TOKENIZER_TURNT_TESTS})
  get_filename_component(TEST_NAME ${TOKENIZER_TURNT_TEST} NAME_WLE)
  get_filename_component(TEST_FILENAME ${TOKENIZER_TURNT_TEST} NAME)
  add_test(NAME tokenizer-${TEST_NAME}
           WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/src/turnt_tests/tokenizer
           COMMAND turnt --command_dir ${CMAKE_BINARY_DIR}/src --diff ${TEST_FILENAME})
endforeach()
