set(LIBRARY_TARGET_NAME vmwriter)

set(${LIBRARY_TARGET_NAME}_SRCS
    vmwriter.cpp
    vmwriter.h
)

add_library(${LIBRARY_TARGET_NAME} ${${LIBRARY_TARGET_NAME}_SRCS})

target_include_directories(${LIBRARY_TARGET_NAME} PRIVATE ..)
