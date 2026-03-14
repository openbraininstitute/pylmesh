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

if(BUILD_PYTHON)
    # Try to import all Python components potentially needed by nanobind
    find_package(Python 3.8
        REQUIRED COMPONENTS Interpreter Development.Module
        OPTIONAL_COMPONENTS Development.SABIModule)
    list(APPEND CMAKE_PREFIX_PATH "${NANOBIND_DIR}")

    # Now actually load nanobind
    find_package(nanobind CONFIG REQUIRED)

    if(nanobind_FOUND)
        message(STATUS "Found nanobind")
    else()
        message(FATAL_ERROR "nanobind not found")
    endif()
endif()