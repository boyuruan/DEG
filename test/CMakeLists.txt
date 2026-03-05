set(CMAKE_CXX_STANDARD 14)

add_executable(main main.cpp)
target_include_directories(main PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(main ${PROJECT_NAME})
