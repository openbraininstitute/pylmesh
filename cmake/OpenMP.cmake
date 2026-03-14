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

if(NOT ENABLE_OPENMP)
    message(STATUS "OpenMP disabled by user")
    return()
endif()

include(CheckCXXCompilerFlag)

# ---- macOS + AppleClang ------------------------------------------------------
if(APPLE AND CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")

    if(EXISTS "/opt/homebrew/opt/libomp")
        set(OpenMP_ROOT "/opt/homebrew/opt/libomp")
    elseif(EXISTS "/usr/local/opt/libomp")
        set(OpenMP_ROOT "/usr/local/opt/libomp")
    else()
        message(FATAL_ERROR
            "AppleClang detected, but libomp was not found.\n"
            "Install it with: brew install libomp")
    endif()

    find_library(OPENMP_RUNTIME_LIBRARY
        NAMES omp libomp
        PATHS "${OpenMP_ROOT}/lib"
        NO_DEFAULT_PATH
    )

    if(NOT OPENMP_RUNTIME_LIBRARY)
        message(FATAL_ERROR "OpenMP runtime library not found under: ${OpenMP_ROOT}/lib")
    endif()

    add_compile_options(-Xpreprocessor -fopenmp)
    include_directories("${OpenMP_ROOT}/include")
    link_libraries("${OPENMP_RUNTIME_LIBRARY}")

    set(PYLMESH_USE_OPENMP TRUE)

    message(STATUS "Using AppleClang + libomp from ${OpenMP_ROOT}")

# ---- Other platforms ---------------------------------------------------------
else()

    find_package(OpenMP QUIET COMPONENTS CXX)

    if(OpenMP_CXX_FOUND)
        set(PYLMESH_USE_OPENMP TRUE)
        message(STATUS "Found OpenMP via CMake")

    else()

        if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
            check_cxx_compiler_flag("-fopenmp" COMPILER_SUPPORTS_FOPENMP)

            if(COMPILER_SUPPORTS_FOPENMP)
                add_compile_options(-fopenmp)
                add_link_options(-fopenmp)

                set(PYLMESH_USE_OPENMP TRUE)

                message(STATUS "Using manual GCC OpenMP fallback")

            else()
                message(FATAL_ERROR "GCC detected but OpenMP not supported")
            endif()

        elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")

            check_cxx_compiler_flag("-fopenmp" COMPILER_SUPPORTS_FOPENMP)

            if(COMPILER_SUPPORTS_FOPENMP)
                add_compile_options(-fopenmp)
                add_link_options(-fopenmp)

                set(PYLMESH_USE_OPENMP TRUE)

                message(STATUS "Using manual Clang OpenMP fallback")

            else()
                message(FATAL_ERROR
                    "Clang detected but OpenMP not found.\n"
                    "Install libomp.")
            endif()

        else()

            message(FATAL_ERROR
                "OpenMP configuration failed for compiler: ${CMAKE_CXX_COMPILER_ID}")

        endif()

    endif()

endif()