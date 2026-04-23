# ####################################################################################################
# # Copyright (c) 2026
# # Open Brain Institute <https://www.openbraininstitute.org/>
# #
# # For complete list of authors, please see AUTHORS.md
# #
# # Licensed under the Apache License, Version 2.0 (the "License");
# # you may not use this file except in compliance with the License.
# # You may obtain a copy of the License at
# #
# #     http://www.apache.org/licenses/LICENSE-2.0
# #
# # Unless required by applicable law or agreed to in writing, software distributed under
# # the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# # KIND, either express or implied.
# # See the License for the specific language governing permissions and
# # limitations under the License.
# ####################################################################################################

# set(SOURCES
#     mesh.cpp
#     compressed_mesh.cpp
#     quantized_mesh.cpp
#     loader.cpp
#     exporter.cpp
#     loaders/obj_loader.cpp
#     loaders/stl_loader.cpp
#     loaders/ply_loader.cpp
#     loaders/off_loader.cpp
#     loaders/gltf_loader.cpp
#     exporters/obj_exporter.cpp
#     exporters/stl_exporter.cpp
#     exporters/ply_exporter.cpp
#     exporters/off_exporter.cpp
#     exporters/gltf_exporter.cpp
# )

# add_library(pylmesh ${SOURCES})

# set_target_properties(pylmesh PROPERTIES POSITION_INDEPENDENT_CODE ON)

# target_include_directories(pylmesh
#     PUBLIC
#         $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
#         $<INSTALL_INTERFACE:include>
# )

# # Link all dependencies at the end
# if(PYLMESH_USE_OPENMP)
#     target_link_libraries(pylmesh PUBLIC OpenMP::OpenMP_CXX)
#     target_compile_definitions(pylmesh PUBLIC PYLMESH_USE_OPENMP)
# endif()

# if(PYLMESH_USE_GLM)
#     target_link_libraries(pylmesh PUBLIC glm)
#     target_compile_definitions(pylmesh PUBLIC PYLMESH_USE_GLM)
# endif()

# if(PYLMESH_USE_JSON)
#     target_link_libraries(pylmesh PUBLIC nlohmann_json::nlohmann_json)
#     target_compile_definitions(pylmesh PUBLIC PYLMESH_USE_JSON)
# endif()

# if(PYLMESH_USE_HDF5)
#     target_link_libraries(pylmesh PUBLIC ${HDF5_LIBRARIES})
#     target_include_directories(pylmesh PUBLIC ${HDF5_INCLUDE_DIRS})
#     target_compile_definitions(pylmesh PUBLIC PYLMESH_USE_HDF5)
# endif()

# if(PYLMESH_USE_TINYGLTF)
#     target_link_libraries(pylmesh PUBLIC tinygltf)
#     target_compile_definitions(pylmesh PUBLIC PYLMESH_USE_TINYGLTF)
# endif()

# if(PYLMESH_USE_DRACO)
#     if(TARGET draco)
#         target_link_libraries(pylmesh PUBLIC draco)
#     elseif(TARGET draco::draco)
#         target_link_libraries(pylmesh PUBLIC draco::draco)
#     endif()
#     target_compile_definitions(pylmesh PUBLIC PYLMESH_USE_DRACO)
# endif()

# # Build Python bindings
# if(BUILD_PYTHON)
#     nanobind_add_module(_pylmesh ${CMAKE_SOURCE_DIR}/python/bindings.cpp)
#     target_link_libraries(_pylmesh PRIVATE pylmesh)
#     install(TARGETS _pylmesh LIBRARY DESTINATION . COMPONENT python)
# endif()
