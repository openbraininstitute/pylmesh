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

option(ENABLE_DRACO "Enable Draco compression support" ON)

set(PYLMESH_USE_DRACO FALSE)

if(NOT ENABLE_DRACO)
    message(STATUS "Draco support disabled")
    return()
endif()

# Prefer system package first
find_package(Draco CONFIG QUIET)

# Try pkg-config if CMake package was not found
if(NOT Draco_FOUND)
    find_package(PkgConfig QUIET)
    
    if(PkgConfig_FOUND)
        pkg_check_modules(DRACO QUIET draco)

        if(DRACO_FOUND)
            add_library(draco::draco INTERFACE IMPORTED)
            target_include_directories(draco::draco INTERFACE ${DRACO_INCLUDE_DIRS})
            target_link_libraries(draco::draco INTERFACE ${DRACO_LINK_LIBRARIES})
            set(Draco_FOUND TRUE)
        endif()
    endif()
endif()

# Fallback: fetch Draco
if(NOT Draco_FOUND)
    message(STATUS "Draco not found via find_package or pkg-config. Fetching with FetchContent.")
  
    include(FetchContent)

    FetchContent_Declare(
        draco
        GIT_REPOSITORY https://github.com/google/draco.git
        GIT_TAG        1.5.7
        GIT_SHALLOW    TRUE
    )

    set(DRACO_TRANSCODER_SUPPORTED OFF CACHE BOOL "" FORCE)
    set(DRACO_TESTS OFF CACHE BOOL "" FORCE)
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

    # Suppress Draco's CMP0148 warning (FindPythonInterp/FindPythonLibs removed)
    # CMAKE_POLICY_DEFAULT propagates into FetchContent subprojects unlike cmake_policy()
    set(CMAKE_POLICY_DEFAULT_CMP0148 OLD CACHE STRING "" FORCE)

    FetchContent_MakeAvailable(draco)

    # Draco's FetchContent targets don't export include dirs properly;
    # add the source include path so `#include "draco/..."` resolves.
    # Use BUILD_INTERFACE to avoid CMake install-time validation errors.
    # Draco's FetchContent targets don't export include dirs properly.
    # Source headers are in draco-src/src/, generated headers (draco_features.h)
    # are in ${CMAKE_BINARY_DIR}/. Both are needed for #include "draco/...".
    set(_draco_inc
        $<BUILD_INTERFACE:${draco_SOURCE_DIR}/src>
        $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}>
    )
    if(TARGET draco_static)
        target_include_directories(draco_static PUBLIC ${_draco_inc})
        set(Draco_FOUND TRUE)
    elseif(TARGET draco)
        target_include_directories(draco PUBLIC ${_draco_inc})
        set(Draco_FOUND TRUE)
    endif()
endif()

if(Draco_FOUND)
    set(PYLMESH_USE_DRACO TRUE)
    message(STATUS "Found Draco")
else()
    message(WARNING "Draco was requested, but could not be found or built. Draco disabled.")
endif()