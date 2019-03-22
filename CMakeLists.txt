cmake_minimum_required(VERSION 3.2)
project(3DApp)

set (CMAKE_C_STANDARD 99)
set (CMAKE_CXX_STANDARD 11)
set (CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (CMAKE_CL_64)
    add_definitions("-DARCH_64")
endif ()

if (WIN32)
    add_definitions("-DPLATFORM_WIN")
endif ()

add_subdirectory(lib)
add_subdirectory(src)

# add_executable(${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/main.cpp)