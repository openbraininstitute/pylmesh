####################################################################################################
# Copyright (c) 2025 - 2026
# Open Brain Institute <https://www.openbraininstitute.org/>
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

option(ENABLE_ZSTD "Enable Zstandard compression" ON)

set(PYLMESH_USE_ZSTD FALSE)

if(ENABLE_ZSTD)
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(ZSTD QUIET libzstd)
    endif()

    if(NOT ZSTD_FOUND)
        find_library(ZSTD_LIBRARY NAMES zstd)
        find_path(ZSTD_INCLUDE_DIR zstd.h)
        if(ZSTD_LIBRARY AND ZSTD_INCLUDE_DIR)
            set(ZSTD_FOUND TRUE)
        endif()
    endif()

    if(ZSTD_FOUND)
        set(PYLMESH_USE_ZSTD TRUE)
        message(STATUS "Found Zstd")
    else()
        message(STATUS "Zstd not found, fetching with FetchContent...")
        include(FetchContent)
        FetchContent_Declare(
            zstd
            GIT_REPOSITORY https://github.com/facebook/zstd.git
            GIT_TAG        v1.5.6
            GIT_SHALLOW    TRUE
            SOURCE_SUBDIR  build/cmake
        )
        set(ZSTD_BUILD_PROGRAMS OFF CACHE BOOL "" FORCE)
        set(ZSTD_BUILD_TESTS OFF CACHE BOOL "" FORCE)
        set(ZSTD_BUILD_SHARED OFF CACHE BOOL "" FORCE)
        set(ZSTD_BUILD_STATIC ON CACHE BOOL "" FORCE)
        FetchContent_MakeAvailable(zstd)
        set(PYLMESH_USE_ZSTD TRUE)
        message(STATUS "Found Zstd (FetchContent)")
    endif()
else()
    message(STATUS "Zstd disabled")
endif()
