if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    add_definitions("-DTRACE_MEMORY_ALLOCATION")
    message(STATUS "Configuration ${CMAKE_BUILD_TYPE}")
endif ()

add_subdirectory(src)