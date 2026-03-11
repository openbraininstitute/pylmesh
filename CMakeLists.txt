####################################################################################################
# Copyright (c) 2026
# Open Brain Institute <https://www.openbraininstitute.org/>
#
# For complete list of authors, please see AUTHORS.md
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software distributed under
# the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
####################################################################################################

cmake_minimum_required(VERSION 3.15)
project(pylmesh VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build options, set ON by default 
option(BUILD_EXAMPLES "Build examples" ON)
option(BUILD_PYTHON "Build Python bindings" ON)
option(ENABLE_GLTF "Enable GLTF/GLB support" ON)
option(ENABLE_DRACO "Enable Draco compression for GLB" ON)

# Add cmake modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# Add CMake customized modules
set(CMAKE_MODULE_PATH  ${CMAKE_SOURCE_DIR}/cmake)
include(OpenMP)
include(GLM)
include(nlohmann_json)
include(HDF5)
include(TinyGLTF)
include(Draco)
include(nanobind)

# Build library
add_subdirectory(src)

# Build examples
if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()