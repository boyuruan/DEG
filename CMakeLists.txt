# -----------------------------------------------------------------------------
# DEG: Dynamic Edge Navigation Graph
# -----------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.5)
project(STKQ LANGUAGES C CXX)

# Export compile commands for IDE / clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# -----------------------------------------------------------------------------
# C++ standard
# -----------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# -----------------------------------------------------------------------------
# Include directories
# -----------------------------------------------------------------------------
include_directories(
    ${PROJECT_SOURCE_DIR}/include
)

# -----------------------------------------------------------------------------
# OpenMP
# -----------------------------------------------------------------------------
find_package(OpenMP REQUIRED)
if (OPENMP_FOUND)
    set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
else()
    message(FATAL_ERROR "OpenMP support required but not found")
endif()

# -----------------------------------------------------------------------------
# Boost
# -----------------------------------------------------------------------------
# Optional: set BOOST_ROOT for custom installs
# set(BOOST_ROOT "/path/to/boost")
find_package(Boost REQUIRED)
if (Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    link_directories(${Boost_LIBRARY_DIRS})
endif()

# -----------------------------------------------------------------------------
# Compiler options (compile only; no linker flags here)
# -----------------------------------------------------------------------------
add_compile_options(
    -O2
    -march=native
    -Wall
)
add_definitions(-DINFO)

# -----------------------------------------------------------------------------
# Subdirectories
# -----------------------------------------------------------------------------
add_subdirectory(src)
add_subdirectory(test)
