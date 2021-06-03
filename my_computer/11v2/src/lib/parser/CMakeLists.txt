set(LIBRARY_TARGET_NAME parser)

set(${LIBRARY_TARGET_NAME}_SRCS
    ast.cpp
    parser.cpp

    ast.h
    parser.h
)

add_library(${LIBRARY_TARGET_NAME} ${${LIBRARY_TARGET_NAME}_SRCS})

target_include_directories(${LIBRARY_TARGET_NAME} PRIVATE ..)
