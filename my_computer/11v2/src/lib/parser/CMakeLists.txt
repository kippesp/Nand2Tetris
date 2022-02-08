set(LIBRARY_TARGET_NAME parser)

set(${LIBRARY_TARGET_NAME}_SRCS
    ast.cpp
    ast.h
    parser.cpp
    parser.h
)

add_library(${LIBRARY_TARGET_NAME} ${${LIBRARY_TARGET_NAME}_SRCS})
set_target_properties(${LIBRARY_TARGET_NAME} PROPERTIES FOLDER libs)

target_include_directories(${LIBRARY_TARGET_NAME} PRIVATE ..)

# Add turnt tests for parser phase

set(ENV{CTEST_OUTPUT_ON_FAILURE} 1)
file(GLOB PARSER_TURNT_TESTS ${CMAKE_SOURCE_DIR}/src/turnt_tests/ast/*.jack)
foreach(PARSER_TURNT_TEST ${PARSER_TURNT_TESTS})
  get_filename_component(TEST_NAME ${PARSER_TURNT_TEST} NAME_WLE)
  get_filename_component(TEST_FILENAME ${PARSER_TURNT_TEST} NAME)
  add_test(NAME parser-${TEST_NAME}
           WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/src/turnt_tests/ast
           COMMAND turnt --command_dir ${CMAKE_BINARY_DIR}/src --diff ${TEST_FILENAME})
endforeach()
