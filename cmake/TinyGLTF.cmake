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

set(PYLMESH_USE_TINYGLTF FALSE)

if(ENABLE_GLTF)
    # Try to find system tinygltf first
    find_package(tinygltf QUIET)
    
    if(NOT tinygltf_FOUND)
        message(STATUS "tinygltf not found, fetching with FetchContent...")
        
        include(FetchContent)
        FetchContent_Declare(
            tinygltf
            GIT_REPOSITORY https://github.com/syoyo/tinygltf.git
            GIT_TAG        v2.8.21
            GIT_SHALLOW    TRUE
        )
        
        FetchContent_MakeAvailable(tinygltf)
    endif()
    
    set(PYLMESH_USE_TINYGLTF TRUE)
    message(STATUS "Found tinygltf")
else()
    message(STATUS "GLTF support disabled")
endif()
