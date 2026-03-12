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

option(ENABLE_OPENMP "Enable OpenMP support" ON)

set(PYLMESH_USE_OPENMP FALSE)

if(ENABLE_OPENMP)
    find_package(OpenMP)

    if(OpenMP_CXX_FOUND)
        set(PYLMESH_USE_OPENMP TRUE)
        message(STATUS "Found OpenMP: ${OpenMP_CXX_VERSION}")
        add_compile_definitions(PYLMESH_USE_OPENMP)
    else()
        message(STATUS "OpenMP not found - using serial fallback")
    endif()
else()
    message(STATUS "OpenMP disabled - using serial fallback")
endif()