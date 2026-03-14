set(PYLMESH_USE_TINYGLTF FALSE)

if(NOT ENABLE_GLTF)
    message(STATUS "GLTF support disabled")
    return()
endif()

find_package(tinygltf QUIET)

if(tinygltf_FOUND)
    set(PYLMESH_USE_TINYGLTF TRUE)
    message(STATUS "Found tinygltf")
else()

    message(STATUS "tinygltf not found, fetching with FetchContent...")

    include(FetchContent)

    FetchContent_Declare(
        tinygltf
        GIT_REPOSITORY https://github.com/syoyo/tinygltf.git
        GIT_TAG v2.8.21
        GIT_SHALLOW TRUE
    )

    cmake_policy(PUSH)
    cmake_policy(SET CMP0169 OLD)

    FetchContent_GetProperties(tinygltf)
    if(NOT tinygltf_POPULATED)
        FetchContent_Populate(tinygltf)
    endif()

    cmake_policy(POP)

    # tinygltf is header-only; create an interface target with the include path
    if(NOT TARGET tinygltf)
        add_library(tinygltf INTERFACE)
        target_include_directories(tinygltf INTERFACE ${tinygltf_SOURCE_DIR})
    endif()

    set(PYLMESH_USE_TINYGLTF TRUE)
    message(STATUS "Found tinygltf")
endif()