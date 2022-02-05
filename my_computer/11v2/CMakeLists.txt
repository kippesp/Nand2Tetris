# See https://github.com/google/benchmark/issues/933
# for applying cstd
#
# https://github.com/StableCoder/cmake-scripts
# for cmake scripts
#
# https://github.com/avast/retdec/blob/master/CMakeLists.txt
# for rpath
# and other cmake stuff
#
# https://github.com/ainfosec/ci_helloworld
# exampe github setup for CI
#
# https://github.com/hakuch/CMakeCooking
# dependency installs
#
# https://github.com/benvenutti/foo
# old github features test
#
#
#
#
#
# https://snorristurluson.github.io/Catch2/
# testing blog
#
#

cmake_minimum_required(VERSION 3.13)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

project(jfcl LANGUAGES CXX)

enable_testing()

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    if (NOT MSVC)
        add_compile_options(-O0)
    endif()
endif()

# cmake 3.8
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

message(STATUS "Compiler ID: " ${CMAKE_CXX_COMPILER_ID})

set(RUN_CLANG_TIDY OFF)

if (${RUN_CLANG_TIDY})
  #set(CMAKE_CXX_CLANG_TIDY clang-tidy -header-filter=.* -checks=-*,modernize-use-*' -fix)
  #set(CMAKE_CXX_CLANG_TIDY clang-tidy -checks=-*,readability-*,modernize-use-*)
  set(CMAKE_CXX_CLANG_TIDY clang-tidy
    -checks=-*,readability-braces-around-statements,readability-qualified-auto,readability-else-after-return,readability-redundant-member-init,readability-convert-member-functions-to-static
    -fix
    )
  #readability-braces-around-statements
  #readability-qualified-auto
  #readability-else-after-return
  #readability-redundant-member-init
  #readability-convert-member-functions-to-static
  #set(CMAKE_CXX_CLANG_TIDY clang-tidy -checks=-*,readability-*)
endif()

# see: https://trac.macports.org/ticket/62426
#if (NOT MSVC)
#    set(CMAKE_BUILD_RPATH "/Users/kippesp/usr/opt/llvm-12.0.1/lib")
#endif()

#add_compile_options(-stdlib=libc++)

#add_compile_options(-stdlib=libc++)
#add_link_options(-stdlib=libc++ -lc++abi)

# Use this with new tools
#set(CMAKE_BUILD_RPATH "/Users/kippesp/usr/opt/llvm-13.0.1-test/lib")
#add_link_options(-L/Users/kippesp/usr/opt/llvm-13.0.1-test/lib)

# cmake 3.13
# TODO: target link options
# TODO: clang version 9 or better
#add_link_options(-stdlib=libc++ -lc++abi)

# Available in newer clang
#add_compile_options(-Wno-c99-extensions)

if (NOT MSVC)
    add_compile_options(-Wall)
    add_compile_options(-Wextra)
    add_compile_options(-Wimplicit-fallthrough)

    if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        add_compile_options(-ferror-limit=10)

        add_compile_options(-Weverything)
        add_compile_options(-Wno-c++98-compat)
        add_compile_options(-Wno-c++98-compat-pedantic)
        add_compile_options(-Wno-padded)

        add_compile_options(-Wshorten-64-to-32)
        add_compile_options(-Wextra-semi)
        add_compile_options(-Wzero-as-null-pointer-constant)
        add_compile_options(-Wshadow-uncaptured-local)
        add_compile_options(-Wcovered-switch-default)
        add_compile_options(-Wsign-conversion)
        add_compile_options(-Wcovered-switch-default)
        add_compile_options(-Wshadow)
        add_compile_options(-Wshadow-field)

        #add_compile_options(-fsanitize=memory)
        #add_link_options(-fsanitize=memory)

        #add_compile_options(-fsanitize=undefined)
        #add_link_options(-fsanitize=undefined)

        #add_compile_options(-fsanitize=address)
        #add_link_options(-fsanitize=address)
        add_compile_options(-D _LIBCPP_NODISCARD_EXT)
    else()
    endif()

    # support c++17 filesystem
    if (USE_LEGACY_CLIARGS)
        # Used with older configs
        add_compile_options(-DLEGACY_CLIARGS)
    else()
        if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
            if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.0)
                message(FATAL_ERROR
                    "Limited std::filesystem support with current compiler configuration; "
                    "either use a newer version of clang, gcc, or current clang "
                    "with -DLEGACY_CLIARGS specified")
            else()
                #add_compile_options(-stdlib=libc++)
                #add_link_options(-stdlib=libc++ -lc++abi)
            endif()
        endif()
    endif()
endif()

add_compile_options(-D_LIBCPP_DISABLE_AVAILABILITY)

# Version of catch2 being used contains several '__' variables
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options(-Wno-reserved-identifier)
endif()

#if (NOT MSVC)
#    add_link_options(-L/Users/kippesp/usr/opt/llvm-12.0.1/lib)
#endif()

# For this project, we're not interested in some things
if (MSVC)
    add_compile_options(-D _CRT_SECURE_NO_WARNINGS)
endif()

if (NOT MSVC)
    add_compile_options(-Wpedantic)
endif()

add_subdirectory(src)
