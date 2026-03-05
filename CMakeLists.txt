cmake_minimum_required(VERSION 3.15)
project(pylmesh VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_EXAMPLES "Build examples" ON)
option(BUILD_TESTS "Build tests" ON)
option(BUILD_PYTHON "Build Python bindings" ON)

add_subdirectory(src)

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if(BUILD_TESTS)
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
