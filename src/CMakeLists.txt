set(SOURCES
    mesh.cpp
    loader.cpp
    loaders/obj_loader.cpp
    loaders/stl_loader.cpp
    loaders/ply_loader.cpp
)

add_library(pylmesh ${SOURCES})

set_target_properties(pylmesh PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_include_directories(pylmesh
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)
