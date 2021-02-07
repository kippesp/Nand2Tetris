# For PCH technique, see http://mochan.info/c++/2019/11/12/pre-compiled-headers-gcc-clang-cmake.html

set(JFCL_TEST_PROJECT_NAME testjfcl)

# Compile the catch2 main() into a separate library
# This code generally will never change
add_library(${JFCL_TEST_PROJECT_NAME}_main STATIC
    main.cpp
    catch.hpp
)

add_executable(${JFCL_TEST_PROJECT_NAME}
    catch.hpp
    mock_reader.h
    test_parse_tree.cpp
    test_tokenizer.cpp
    test_vmwriter.cpp
    jack_sources.cpp
    jack_sources.h
)

target_include_directories(${JFCL_TEST_PROJECT_NAME} PRIVATE ../lib)
target_link_libraries(${JFCL_TEST_PROJECT_NAME} ${JFCL_TEST_PROJECT_NAME}_main jfclmain)

# TODO: This has a problem where if the PCH source header is later changed,
# the compile will fail if 'make clean' isn't issued.

# Precompile the catch.hpp header to save about half the time (or ~0.5 sec)
target_compile_definitions(${JFCL_TEST_PROJECT_NAME} PRIVATE CATCH_CONFIG_FAST_COMPILE CATCH_CONFIG_DISABLE_MATCHERS)
target_precompile_headers(${JFCL_TEST_PROJECT_NAME} PRIVATE catch.hpp)
