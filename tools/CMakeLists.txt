if (${RENDER_API} STREQUAL "D3D9")
    add_definitions("-DRENDERAPI_DX9")
    include_directories("${PROJECT_SOURCE_DIR}/lib/dependencies/d3d")
elseif (${RENDER_API} STREQUAL "D3D11")
    add_definitions("-DRENDERAPI_DX11")
endif ()

include_directories("${PROJECT_SOURCE_DIR}/lib/include")
add_subdirectory(ShaderPrecacher)