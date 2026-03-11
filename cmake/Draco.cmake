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

option(ENABLE_DRACO "Enable Draco compression support" ON)

set(PYLMESH_USE_DRACO FALSE)

if(ENABLE_DRACO)
  # Prefer system package first
  find_package(Draco CONFIG QUIET)

  # Try pkg-config if CMake config not found
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

    FetchContent_MakeAvailable(draco)
    
    # FetchContent creates the draco target, mark as found
  endif()

  # After either path, check for a usable target
  if(Draco_FOUND)
    set(PYLMESH_USE_DRACO TRUE)
    if(VERBOSE)
        message(STATUS "Found Draco. ${Draco_DIR}")
    else(VERBOSE)
        message(STATUS "Found Draco")
    endif(VERBOSE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DPYLMESH_USE_DRACO")
  else()
    message(WARNING "Draco was requested, but could not be found or built. Draco disabled.")
  endif()
endif()