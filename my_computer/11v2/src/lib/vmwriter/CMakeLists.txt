set(LIBRARY_TARGET_NAME vmwriter)

set(${LIBRARY_TARGET_NAME}_SRCS
    class_descr.h
    program.h
    semantic_exception.h
    subroutine_descr.h
    symbol_table.h
    vmwriter.h

    class_descr.cpp
    semantic_exception.cpp
    subroutine_descr.cpp
    symbol_table.cpp
    vmwriter.cpp
)

add_library(${LIBRARY_TARGET_NAME} ${${LIBRARY_TARGET_NAME}_SRCS})
set_target_properties(${LIBRARY_TARGET_NAME} PROPERTIES FOLDER libs)

target_include_directories(${LIBRARY_TARGET_NAME} PRIVATE ..)

# Add turnt tests for vmwriter phase

set(ENV{CTEST_OUTPUT_ON_FAILURE} 1)
file(GLOB PARSER_TURNT_TESTS ${CMAKE_SOURCE_DIR}/src/turnt_tests/vmwriter/*/*.jack)
foreach(PARSER_TURNT_TEST ${PARSER_TURNT_TESTS})
  get_filename_component(TEST_FULL_DIR ${PARSER_TURNT_TEST} DIRECTORY)
  get_filename_component(TEST_DIR ${TEST_FULL_DIR} NAME)
  get_filename_component(TEST_NAME ${PARSER_TURNT_TEST} NAME_WLE)
  get_filename_component(TEST_FILENAME ${PARSER_TURNT_TEST} NAME)
  add_test(NAME vmwriter-${TEST_DIR}-${TEST_NAME}
           WORKING_DIRECTORY ${TEST_FULL_DIR}
           COMMAND turnt --command_dir ${CMAKE_BINARY_DIR}/src --diff ${TEST_FILENAME})
endforeach()
