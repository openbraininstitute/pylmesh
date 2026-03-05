cmake_minimum_required(VERSION 3.15)
project(pylmesh VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build options
option(BUILD_EXAMPLES "Build examples" ON)
option(BUILD_TESTS "Build tests" ON)
option(BUILD_PYTHON "Build Python bindings" ON)
option(ENABLE_GLTF "Enable GLTF/GLB support" ON)
option(ENABLE_DRACO "Enable Draco compression for GLB" ON)

# Add cmake modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# Download dependencies
include(DownloadDependencies)

# Setup Draco
include(SetupDraco)

# Build library
add_subdirectory(src)

# Build examples
if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# Build tests
if(BUILD_TESTS AND EXISTS "${CMAKE_SOURCE_DIR}/tests")
    enable_testing()
    add_subdirectory(tests)
endif()

# Setup Python bindings
include(SetupPython)
