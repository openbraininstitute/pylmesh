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

option(ENABLE_JSON "Enable nlohmann_json support" ON)

set(PYLMESH_USE_JSON FALSE)

if(ENABLE_JSON)
    find_package(nlohmann_json QUIET)

    if(nlohmann_json_FOUND)
        set(PYLMESH_USE_JSON TRUE)
        message(STATUS "Found nlohmann_json")
    else()
        message(STATUS "nlohmann_json not found by find_package. Trying manual search...")

        find_path(NLOHMANN_JSON_INCLUDE_DIR nlohmann/json.hpp
            HINTS
                /usr/include
                /usr/local/include
        )

        if(NLOHMANN_JSON_INCLUDE_DIR)
            set(PYLMESH_USE_JSON TRUE)
            message(STATUS "Found nlohmann_json at: ${NLOHMANN_JSON_INCLUDE_DIR}")
            
            add_library(nlohmann_json INTERFACE IMPORTED)
            target_include_directories(nlohmann_json INTERFACE ${NLOHMANN_JSON_INCLUDE_DIR})
        else()
            message(STATUS "nlohmann_json not found")
        endif()
    endif()
else()
    message(STATUS "nlohmann_json disabled")
endif()