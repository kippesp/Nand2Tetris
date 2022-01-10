set(LIBRARY_TARGET_NAME vmwriter)

set(${LIBRARY_TARGET_NAME}_SRCS
    class_descr.h
    program.h
    semantic_exception.h
    subroutine_descr.h
    symbol_table.h
    vmwriter.h

    class_descr.cpp
    subroutine_descr.cpp
    symbol_table.cpp
    vmwriter.cpp
)

add_library(${LIBRARY_TARGET_NAME} ${${LIBRARY_TARGET_NAME}_SRCS})
set_target_properties(${LIBRARY_TARGET_NAME} PROPERTIES FOLDER libs)

target_include_directories(${LIBRARY_TARGET_NAME} PRIVATE ..)
