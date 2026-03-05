cmake_minimum_required(VERSION 3.15)
project(pylmesh VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_EXAMPLES "Build examples" ON)
option(BUILD_TESTS "Build tests" ON)
option(BUILD_PYTHON "Build Python bindings" ON)
option(ENABLE_GLTF "Enable GLTF/GLB support" ON)
option(ENABLE_DRACO "Enable Draco compression for GLB" ON)

# Download dependencies if GLTF is enabled
if(ENABLE_GLTF)
    set(EXTERNAL_DIR "${CMAKE_SOURCE_DIR}/include/pylmesh/external")
    file(MAKE_DIRECTORY ${EXTERNAL_DIR})
    
    # Download tinygltf
    if(NOT EXISTS "${EXTERNAL_DIR}/tiny_gltf.h")
        message(STATUS "Downloading tinygltf...")
        file(DOWNLOAD
            "https://raw.githubusercontent.com/syoyo/tinygltf/release/tiny_gltf.h"
            "${EXTERNAL_DIR}/tiny_gltf.h"
            SHOW_PROGRESS)
    endif()
    
    # Download nlohmann json
    if(NOT EXISTS "${EXTERNAL_DIR}/json.hpp")
        message(STATUS "Downloading nlohmann/json...")
        file(DOWNLOAD
            "https://raw.githubusercontent.com/nlohmann/json/develop/single_include/nlohmann/json.hpp"
            "${EXTERNAL_DIR}/json.hpp"
            SHOW_PROGRESS)
    endif()
    
    # Download stb_image
    if(NOT EXISTS "${EXTERNAL_DIR}/stb_image.h")
        message(STATUS "Downloading stb_image...")
        file(DOWNLOAD
            "https://raw.githubusercontent.com/nothings/stb/master/stb_image.h"
            "${EXTERNAL_DIR}/stb_image.h"
            SHOW_PROGRESS)
    endif()
    
    # Download stb_image_write
    if(NOT EXISTS "${EXTERNAL_DIR}/stb_image_write.h")
        message(STATUS "Downloading stb_image_write...")
        file(DOWNLOAD
            "https://raw.githubusercontent.com/nothings/stb/master/stb_image_write.h"
            "${EXTERNAL_DIR}/stb_image_write.h"
            SHOW_PROGRESS)
    endif()
endif()

# Setup Draco if enabled
set(PYLMESH_USE_DRACO FALSE)
if(ENABLE_DRACO AND ENABLE_GLTF)
    # Try to find Draco
    find_package(draco CONFIG QUIET)
    
    # Try pkg-config if CMake config not found
    if(NOT draco_FOUND)
        find_package(PkgConfig QUIET)
        if(PkgConfig_FOUND)
            pkg_check_modules(DRACO QUIET draco)
            if(DRACO_FOUND)
                add_library(draco::draco INTERFACE IMPORTED)
                target_include_directories(draco::draco INTERFACE ${DRACO_INCLUDE_DIRS})
                target_link_libraries(draco::draco INTERFACE ${DRACO_LINK_LIBRARIES})
                set(draco_FOUND TRUE)
            endif()
        endif()
    endif()
    
    # Fetch Draco if not found
    if(NOT draco_FOUND)
        message(STATUS "Draco not found. Fetching from GitHub...")
        include(FetchContent)
        FetchContent_Declare(
            draco
            GIT_REPOSITORY https://github.com/google/draco.git
            GIT_TAG 1.5.7
            GIT_SHALLOW TRUE
        )
        
        set(DRACO_TRANSCODER_SUPPORTED OFF CACHE BOOL "" FORCE)
        set(DRACO_TESTS OFF CACHE BOOL "" FORCE)
        set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
        
        FetchContent_MakeAvailable(draco)
        set(draco_FOUND TRUE)
    endif()
    
    if(draco_FOUND)
        set(PYLMESH_USE_DRACO TRUE)
        message(STATUS "Draco compression enabled")
    else()
        message(WARNING "Draco requested but not available. GLB will be uncompressed.")
    endif()
endif()

add_subdirectory(src)

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if(BUILD_TESTS AND EXISTS "${CMAKE_SOURCE_DIR}/tests")
    enable_testing()
    add_subdirectory(tests)
endif()

if(BUILD_PYTHON)
    find_package(pybind11 CONFIG QUIET)
    if(pybind11_FOUND)
        pybind11_add_module(_pylmesh python/bindings.cpp)
        target_link_libraries(_pylmesh PRIVATE pylmesh)
        install(TARGETS _pylmesh LIBRARY DESTINATION . COMPONENT python)
        install(FILES python/pylmesh/__init__.py DESTINATION . COMPONENT python)
    endif()
endif()
