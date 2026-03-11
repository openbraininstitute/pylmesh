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

option(ENABLE_GLM "Enable GLM support" ON)

set(PYLMESH_USE_GLM FALSE)

if(ENABLE_GLM)
    find_package(glm QUIET)

    if(glm_FOUND)
        set(PYLMESH_USE_GLM TRUE)
        message(STATUS "Found GLM")
        
        # Create interface target if not already created by find_package
        if(NOT TARGET glm)
            add_library(glm INTERFACE IMPORTED)
            if(GLM_INCLUDE_DIRS)
                target_include_directories(glm INTERFACE ${GLM_INCLUDE_DIRS})
            endif()
        endif()
        
        if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
            add_compile_options(-Wno-error=class-memaccess)
        endif()
    else()
        message(STATUS "GLM not found by find_package. Trying manual search...")

        find_path(GLM_INCLUDE_DIR glm/glm.hpp
            HINTS
                /usr/include
                /usr/local/include
                /opt/local/include
        )

        if(GLM_INCLUDE_DIR)
            set(PYLMESH_USE_GLM TRUE)
            message(STATUS "Found GLM at: ${GLM_INCLUDE_DIR}")
            
            add_library(glm INTERFACE IMPORTED)
            target_include_directories(glm INTERFACE ${GLM_INCLUDE_DIR})
            
            if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
                add_compile_options(-Wno-error=class-memaccess)
            endif()
        else()
            message(STATUS "GLM not found")
        endif()
    endif()
else()
    message(STATUS "GLM disabled")
endif()