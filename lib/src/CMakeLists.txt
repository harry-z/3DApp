include_directories("${PROJECT_SOURCE_DIR}/lib/include")
add_definitions("-DEXPORTS")
add_library(${PROJECT_NAME}_lib SHARED 
    Global.cpp 
    Matrix.cpp
    ./Windows/Display_Windows.cpp)