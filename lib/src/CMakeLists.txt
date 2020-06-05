include_directories("${PROJECT_SOURCE_DIR}/lib/include")
include_directories("${PROJECT_SOURCE_DIR}/lib/dependencies")
add_definitions("-DEXPORTS")

if (${RENDER_API} STREQUAL "D3D9")
    add_definitions("-DRENDERAPI_DX9")
    aux_source_directory(${PROJECT_SOURCE_DIR}/lib/src/Backend/D3D9 RENDER_API_SRC)
    link_libraries(d3d9 dxguid)
    if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
        link_libraries(${PROJECT_SOURCE_DIR}/lib/dependencies/d3d/d3dx9d.lib)
    else ()
        link_libraries(${PROJECT_SOURCE_DIR}/lib/dependencies/d3d/d3dx9.lib)
    endif ()
elseif (${RENDER_API} STREQUAL "D3D11")
    add_definitions("-DRENDERAPI_DX11")
    aux_source_directory(${PROJECT_SOURCE_DIR}/lib/src/Backend/D3D11 RENDER_API_SRC)
    link_libraries(d3d11 d3dcompiler dxgi)
endif ()

if (WIN32)
    add_definitions("-DINPUTAPI_DINPUT")
    aux_source_directory(${PROJECT_SOURCE_DIR}/lib/src/Backend/DInput INPUT_API_SRC)
    aux_source_directory(${PROJECT_SOURCE_DIR}/lib/src/Windows DISPLAY_SRC)
    link_libraries(dinput8)
    if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
        link_libraries(${PROJECT_SOURCE_DIR}/lib/dependencies/png/debug/libpng16.lib)
    endif ()
endif ()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/src)
add_library(${PROJECT_NAME}_lib SHARED 
    Atomsphere.cpp
    Camera.cpp
    CustomGeometryNode.cpp
    File.cpp
    FirstPersonCameraController.cpp
    FreelistAlloc.cpp
    Global.cpp 
    Hashmap.cpp
    HardwareBuffer.cpp
    InputListener.cpp
    JobSystem.cpp
    Log.cpp
    Material.cpp
    MaterialParser.cpp
    Matrix.cpp
    Memory.cpp
    OrbitCameraController.cpp
    PixelFormatUtil.cpp
    Pool.cpp
    RenderNode.cpp
    SceneClipping.cpp
    ScriptParser.cpp
    Shader.cpp
    Stack.cpp
    Str.cpp
    Texture.cpp
    TextureReader.cpp
    Timer.cpp
    Worker.cpp
    3DEngine.cpp
    3DEngineNode.cpp
    ${RENDER_API_SRC}
    ${INPUT_API_SRC}
    ${DISPLAY_SRC}
)