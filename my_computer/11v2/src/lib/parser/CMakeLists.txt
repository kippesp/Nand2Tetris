set(LIBRARY_TARGET_NAME parser)

set(${LIBRARY_TARGET_NAME}_SRCS
    ast.cpp
    ast.h
    parser.cpp
    parser.h
)

add_library(${LIBRARY_TARGET_NAME} ${${LIBRARY_TARGET_NAME}_SRCS})
set_target_properties(${LIBRARY_TARGET_NAME} PROPERTIES FOLDER libs)

target_include_directories(${LIBRARY_TARGET_NAME} PRIVATE ..)
