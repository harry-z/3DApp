if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    add_definitions("-DTRACE_MEMORY_ALLOCATION")
endif ()

add_subdirectory(src)