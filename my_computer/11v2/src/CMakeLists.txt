add_subdirectory(lib)
add_subdirectory(unit_tests)

set(${PROJECT_NAME}_SRCS
    main.cpp
)

add_executable(${PROJECT_NAME} ${${PROJECT_NAME}_SRCS})
target_include_directories(${PROJECT_NAME} PRIVATE lib)

if (NOT MSVC)
    if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        target_link_libraries(${PROJECT_NAME} jfclmain)
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
        target_link_libraries(${PROJECT_NAME} jfclmain)
    else()
        target_link_libraries(${PROJECT_NAME} stdc++fs jfclmain)
    endif()
else()
    target_link_libraries(${PROJECT_NAME} jfclmain)
endif()

add_subdirectory(turnt_tests)
