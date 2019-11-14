if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    add_definitions("-DTRACE_MEMORY_ALLOCATION")
endif ()

include_directories("${PROJECT_SOURCE_DIR}/lib" "${PROJECT_SOURCE_DIR}/lib/include")
add_executable(${PROJECT_NAME} 
    main.cpp
    Test.cpp
)
target_link_libraries(${PROJECT_NAME} ${PROJECT_NAME}_lib)

add_custom_command(TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/lib/dependencies/png/debug/libpng16.dll ${PROJECT_BINARY_DIR}/src/Debug
    COMMENT "Copy libpng"
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/rc ${PROJECT_BINARY_DIR}/src/Debug
    COMMENT "Copy rc"
)