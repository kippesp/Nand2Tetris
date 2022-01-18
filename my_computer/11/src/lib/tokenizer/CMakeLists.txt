set(LIBRARY_TARGET_NAME tokenizer)

set(${LIBRARY_TARGET_NAME}_SRCS
    file_io.cpp
    jack_tokenizer.cpp
    jack_lexical_elements.cpp

    file_io.h
    jack_tokenizer.h
    jack_lexical_elements.h
)

add_library(${LIBRARY_TARGET_NAME} ${${LIBRARY_TARGET_NAME}_SRCS})
set_target_properties(${LIBRARY_TARGET_NAME} PROPERTIES FOLDER libs)
