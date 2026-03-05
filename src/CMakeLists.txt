set(SOURCES
    mesh.cpp
    loader.cpp
    exporter.cpp
    loaders/obj_loader.cpp
    loaders/stl_loader.cpp
    loaders/ply_loader.cpp
    loaders/off_loader.cpp
    loaders/gltf_loader.cpp
    exporters/obj_exporter.cpp
    exporters/stl_exporter.cpp
    exporters/ply_exporter.cpp
    exporters/off_exporter.cpp
    exporters/gltf_exporter.cpp
)

add_library(pylmesh ${SOURCES})

set_target_properties(pylmesh PROPERTIES POSITION_INDEPENDENT_CODE ON)

# Check if tinygltf is available
if(EXISTS "${CMAKE_SOURCE_DIR}/include/pylmesh/external/tiny_gltf.h")
    target_compile_definitions(pylmesh PRIVATE PYLMESH_USE_TINYGLTF)
    target_include_directories(pylmesh PRIVATE ${CMAKE_SOURCE_DIR}/include/pylmesh/external)
endif()

target_include_directories(pylmesh
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)
