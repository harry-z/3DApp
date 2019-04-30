if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    add_definitions("-DTRACE_MEMORY_ALLOCATION")
endif ()

include_directories("${PROJECT_SOURCE_DIR}/lib/dependencies")
add_subdirectory(src)