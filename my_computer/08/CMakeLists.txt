cmake_minimum_required(VERSION 2.8.12)

set(PROJECT_NAME vmt)
project(${PROJECT_NAME} CXX)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Wextra)

if (NOT MSVC)
  add_compile_options(-pedantic)
endif()

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -g")

set(VMT_SRCS
  CMakeLists.txt
  main.cpp
  )

add_executable(${PROJECT_NAME}
  ${VMT_SRCS}
  )

include_directories(
  ${CMAKE_INCLUDE_PATH}
  )

set(CMAKE_SKIP_RPATH true)

#target_link_libraries(${PROJECT_NAME}
#  ""
#  )
