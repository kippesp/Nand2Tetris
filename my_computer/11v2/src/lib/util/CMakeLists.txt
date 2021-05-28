set(LIBRARY_TARGET_NAME util)

set(${LIBRARY_TARGET_NAME}_SRCS
    cli_args.cpp
    text_reader.cpp
    textfile_reader.cpp

    cli_args.h
    text_reader.h
    textfile_reader.h
)

add_library(${LIBRARY_TARGET_NAME} ${${LIBRARY_TARGET_NAME}_SRCS})
