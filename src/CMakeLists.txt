cmake_minimum_required(VERSION 3.5)
set(CMAKE_CXX_STANDARD 11)

# Sources relative to this directory (current path)
file(GLOB_RECURSE CPP_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_library(${PROJECT_NAME} ${CPP_SOURCES})
add_library(${PROJECT_NAME}_s SHARED ${CPP_SOURCES})