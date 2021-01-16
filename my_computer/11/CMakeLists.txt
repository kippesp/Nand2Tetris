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

set(PROJECT_NAME jfcl)
project(${PROJECT_NAME} CXX)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    add_compile_options(-O0)
endif()

# cmake 3.8
#set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

message(STATUS "Compiler ID: " ${CMAKE_CXX_COMPILER_ID})

#set(CMAKE_BUILD_RPATH "/home/kippesp/usr/opt/llvm-10.0.x/lib")

add_compile_options(-Wall -Wextra -Wimplicit-fallthrough)
#add_compile_options(-stdlib=libc++)

# cmake 3.13
#add_link_options(-stdlib=libc++ -lc++abi)

# Available in newer clang
#add_compile_options(-Wno-c99-extensions)

if (NOT MSVC)
    if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        #add_compile_options(-Weverything)
        #add_compile_options(-Wno-c++98-compat)
        #add_compile_options(-Wno-c++98-compat-pedantic)
        #add_compile_options(-Wno-padded)

        add_compile_options(-Wshorten-64-to-32)
        add_compile_options(-Wextra-semi)
        add_compile_options(-Wzero-as-null-pointer-constant)
        add_compile_options(-Wshadow-uncaptured-local)
        add_compile_options(-Wcovered-switch-default)
        add_compile_options(-Wsign-conversion)
        add_compile_options(-Wcovered-switch-default)
        add_compile_options(-Wshadow)
        add_compile_options(-Wshadow-field)

    else()
    endif()

    if (USE_LEGACY_CLIARGS)
        add_compile_options(-DLEGACY_CLIARGS)
    else()
        if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
            # support c++17 filesystem
            add_compile_options(-stdlib=libc++)
        endif()
    endif()
endif()

if (NOT MSVC)
    add_compile_options(-Wpedantic)
endif()

add_subdirectory(src)
