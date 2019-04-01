include_directories("${PROJECT_SOURCE_DIR}/lib/include")
add_definitions("-DEXPORTS")

if (${RENDER_API} MATCHES "D3D9")
    add_definitions("-DRENDERAPI_DX9")
    aux_source_directory(${PROJECT_SOURCE_DIR}/lib/src/Backend/D3D9 RENDER_API_SRC)
    link_libraries(d3d9 d3dcompiler dxguid)
endif ()

add_library(${PROJECT_NAME}_lib SHARED 
    Global.cpp 
    Matrix.cpp
    3DEngine.cpp
    ./Windows/Display_Windows.cpp
    ${RENDER_API_SRC})