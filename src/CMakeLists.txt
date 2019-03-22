include_directories("${PROJECT_SOURCE_DIR}/lib" "${PROJECT_SOURCE_DIR}/lib/include")
add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} ${PROJECT_NAME}_lib)