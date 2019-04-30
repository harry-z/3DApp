set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/src)
add_executable(ShaderPrecacher main.cpp ShaderPrecacher.cpp)
target_link_libraries(ShaderPrecacher ${PROJECT_NAME}_lib)