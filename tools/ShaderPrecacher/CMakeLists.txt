if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    add_definitions("-DTRACE_MEMORY_ALLOCATION" "-DSHADER_DEBUGGING")
    if (${RENDER_API} STREQUAL "D3D9")
        link_libraries(${PROJECT_SOURCE_DIR}/lib/dependencies/d3d/d3dx9d.lib)
    elseif (${RENDER_API} STREQUAL "D3D11")
        link_libraries(d3dcompiler)
    endif ()
else ()
    if (${RENDER_API} STREQUAL "D3D9")      
        link_libraries(${PROJECT_SOURCE_DIR}/lib/dependencies/d3d/d3dx9.lib)
    elseif (${RENDER_API} STREQUAL "D3D11")
        link_libraries(d3dcompiler)
    endif ()
endif ()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/src)
add_executable(ShaderPrecacher main.cpp ShaderPrecacher.cpp)
target_link_libraries(ShaderPrecacher ${PROJECT_NAME}_lib)