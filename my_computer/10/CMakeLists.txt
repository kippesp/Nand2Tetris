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

#set(CMAKE_BUILD_RPATH "/home/kippesp/usr/opt/llvm-10.0.x/lib")

add_compile_options(-Wall -Wextra)
#add_compile_options(-stdlib=libc++)

# cmake 3.13
#add_link_options(-stdlib=libc++ -lc++abi)

# Available in newer clang
#add_compile_options(-Wno-c99-extensions)

if (NOT MSVC)
  add_compile_options(-pedantic)
endif()

add_subdirectory(src)
