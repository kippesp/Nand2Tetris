cmake_minimum_required(VERSION 2.8.12)

include("${CMAKE_SOURCE_DIR}/cmake/c++-standards.cmake")

set(PROJECT_NAME mg)
project(${PROJECT_NAME} CXX)

set(CMAKE_BUILD_TYPE Debug)

cxx_20()

add_compile_options(-Wall -Wextra -Wno-c99-extensions)

# Available in newer clang
add_compile_options(-Wno-c99-extensions)

if (NOT MSVC)
  add_compile_options(-pedantic)
endif()

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -g")

set(JC_SRCS
  CMakeLists.txt
  main.cpp
  )

add_executable(${PROJECT_NAME}
    ${JC_SRCS}
  )

include_directories(
  ${CMAKE_INCLUDE_PATH}
  )

set(CMAKE_SKIP_RPATH true)
