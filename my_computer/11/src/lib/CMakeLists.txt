set(LIBRARY_TARGET_NAME jfclmain)

set(${LIBRARY_TARGET_NAME}_SRCS
    cli_args.cpp
    cli_args.h
)

add_subdirectory(tokenizer)
add_subdirectory(parser)
add_subdirectory(vmwriter)

add_library(${LIBRARY_TARGET_NAME} ${${LIBRARY_TARGET_NAME}_SRCS})

# TODO: PRIVATE works too.
target_link_libraries(${LIBRARY_TARGET_NAME} PUBLIC tokenizer parser vmwriter)
set_target_properties(${LIBRARY_TARGET_NAME} PROPERTIES FOLDER libs)
