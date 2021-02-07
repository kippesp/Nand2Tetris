add_subdirectory(lib)
add_subdirectory(unit_tests)

set(${PROJECT_NAME}_SRCS
    main.cpp
)

add_executable(${PROJECT_NAME} ${${PROJECT_NAME}_SRCS})

target_include_directories(${PROJECT_NAME} PRIVATE lib)

target_link_libraries(${PROJECT_NAME} jfclmain)
