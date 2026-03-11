set(SOURCES
    src/mesh.cpp
    src/loader.cpp
    src/exporter.cpp
    src/loaders/obj_loader.cpp
    src/loaders/stl_loader.cpp
    src/loaders/ply_loader.cpp
    src/loaders/off_loader.cpp
    src/loaders/gltf_loader.cpp
    src/exporters/obj_exporter.cpp
    src/exporters/stl_exporter.cpp
    src/exporters/ply_exporter.cpp
    src/exporters/off_exporter.cpp
    src/exporters/gltf_exporter.cpp
)

add_library(lmesh ${SOURCES})

set_target_properties(lmesh PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_include_directories(lmesh
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Link all dependencies
if(PYLMESH_USE_OPENMP)
    target_link_libraries(lmesh PUBLIC OpenMP::OpenMP_CXX)
    target_compile_definitions(lmesh PUBLIC PYLMESH_USE_OPENMP)
endif()

if(PYLMESH_USE_GLM)
    target_link_libraries(lmesh PUBLIC glm)
    target_compile_definitions(lmesh PUBLIC PYLMESH_USE_GLM)
endif()

if(PYLMESH_USE_JSON)
    target_link_libraries(lmesh PUBLIC nlohmann_json::nlohmann_json)
    target_compile_definitions(lmesh PUBLIC PYLMESH_USE_JSON)
endif()

if(PYLMESH_USE_HDF5)
    target_link_libraries(lmesh PUBLIC ${HDF5_LIBRARIES})
    target_include_directories(lmesh PUBLIC ${HDF5_INCLUDE_DIRS})
    target_compile_definitions(lmesh PUBLIC PYLMESH_USE_HDF5)
endif()

if(PYLMESH_USE_TINYGLTF)
    target_link_libraries(lmesh PUBLIC tinygltf)
    target_compile_definitions(lmesh PUBLIC PYLMESH_USE_TINYGLTF)
endif()

if(PYLMESH_USE_DRACO)
    if(TARGET draco)
        target_link_libraries(lmesh PUBLIC draco)
    elseif(TARGET draco::draco)
        target_link_libraries(lmesh PUBLIC draco::draco)
    endif()
    target_compile_definitions(lmesh PUBLIC PYLMESH_USE_DRACO)
endif()
