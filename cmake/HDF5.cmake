####################################################################################################
# Copyright (c) 2026
# Open Brain Institute <https://www.openbraininstitute.org/>
#
# Author(s): Marwan Abdellah <marwan.abdellah@openbraininstitute.org>
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

option(ENABLE_HDF5 "Enable HDF5 support" ON)

set(PYLMESH_USE_HDF5 FALSE)

if(ENABLE_HDF5)
    find_package(HDF5 COMPONENTS C CXX HL)

    if(HDF5_FOUND)
        set(PYLMESH_USE_HDF5 TRUE)
        message(STATUS "Found HDF5")
        
        # Modern CMake approach - use imported targets
        # HDF5::HDF5 target handles everything
    else()
        message(STATUS "HDF5 not found")
    endif()
else()
    message(STATUS "HDF5 disabled")
endif()