include_directories("${PROJECT_SOURCE_DIR}/lib/include")
add_definitions("-DEXPORTS")

if (${RENDER_API} STREQUAL "D3D9")
    add_definitions("-DRENDERAPI_DX9")
    aux_source_directory(${PROJECT_SOURCE_DIR}/lib/src/Backend/D3D9 RENDER_API_SRC)
    link_libraries(d3d9 d3dcompiler dxguid)
endif ()

if (WIN32)
    add_definitions("-DINPUTAPI_DINPUT")
    aux_source_directory(${PROJECT_SOURCE_DIR}/lib/src/Backend/DInput INPUT_API_SRC)
    aux_source_directory(${PROJECT_SOURCE_DIR}/lib/src/Windows/ DISPLAY_SRC)
    link_libraries(dinput8)
endif ()

add_library(${PROJECT_NAME}_lib SHARED 
    Camera.cpp
    File.cpp
    FreelistAlloc.cpp
    Global.cpp 
    Hashmap.cpp
    InputListener.cpp
    JobSystem.cpp
    Log.cpp
    Matrix.cpp
    Memory.cpp
    Pool.cpp
    Stack.cpp
    Str.cpp
    Timer.cpp
    Worker.cpp
    3DEngine.cpp
    ${RENDER_API_SRC}
    ${INPUT_API_SRC}
    ${DISPLAY_SRC})