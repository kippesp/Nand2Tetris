cmake_minimum_required(VERSION 2.8.12)
project(vmt CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra")

if (NOT MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic")
endif()

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -g")

set(VMT_SRCS
  CMakeLists.txt
  main.cpp
  )

add_executable(vmt
  ${VMT_SRCS}
  )

include_directories(
  ${CMAKE_INCLUDE_PATH}
  )

set(CMAKE_SKIP_RPATH true)

#target_link_libraries(vmt
#  ""
#  )
