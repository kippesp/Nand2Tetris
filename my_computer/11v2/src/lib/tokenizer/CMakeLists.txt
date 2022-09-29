set(LIBRARY_TARGET_NAME tokenizer)

set(${LIBRARY_TARGET_NAME}_SRCS
    jack_tokenizer.cpp
    jack_token.cpp

    jack_tokenizer.h
    jack_token.h
)

add_library(${LIBRARY_TARGET_NAME} ${${LIBRARY_TARGET_NAME}_SRCS})
set_target_properties(${LIBRARY_TARGET_NAME} PROPERTIES FOLDER libs)

target_include_directories(${LIBRARY_TARGET_NAME} PRIVATE ..)
