set(LIBRARY_TARGET_NAME jfclmain)

set(${LIBRARY_TARGET_NAME}_SRCS
    empty.cpp
)

add_subdirectory(util)
add_subdirectory(tokenizer)
add_subdirectory(parser)
add_subdirectory(vmwriter)

add_library(${LIBRARY_TARGET_NAME} ${${LIBRARY_TARGET_NAME}_SRCS})

# TODO: PRIVATE works too.
#target_link_libraries(${LIBRARY_TARGET_NAME} PUBLIC util)
#target_link_libraries(${LIBRARY_TARGET_NAME} PUBLIC util tokenizer)
#target_link_libraries(${LIBRARY_TARGET_NAME} PUBLIC util tokenizer parser)
target_link_libraries(${LIBRARY_TARGET_NAME} PUBLIC util tokenizer parser vmwriter)

set_target_properties(${LIBRARY_TARGET_NAME} PROPERTIES FOLDER libs)
