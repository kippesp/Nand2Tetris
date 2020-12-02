cmake_minimum_required(VERSION 3.13)

set(PROJECT_NAME mg)
project(${PROJECT_NAME} CXX)

set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-Wall -Wextra)

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
