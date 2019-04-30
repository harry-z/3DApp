if (${RENDER_API} STREQUAL "D3D9")
    add_definitions("-DRENDERAPI_DX9")
endif ()

include_directories("${PROJECT_SOURCE_DIR}/lib/include")
add_subdirectory(ShaderPrecacher)