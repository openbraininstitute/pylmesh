# Setup Draco compression library

set(PYLMESH_USE_DRACO FALSE)

if(ENABLE_DRACO AND ENABLE_GLTF)
    # Try to find Draco
    find_package(draco CONFIG QUIET)
    
    # Try pkg-config if CMake config not found
    if(NOT draco_FOUND)
        find_package(PkgConfig QUIET)
        if(PkgConfig_FOUND)
            pkg_check_modules(DRACO QUIET draco)
            if(DRACO_FOUND)
                add_library(draco::draco INTERFACE IMPORTED)
                target_include_directories(draco::draco INTERFACE ${DRACO_INCLUDE_DIRS})
                target_link_libraries(draco::draco INTERFACE ${DRACO_LINK_LIBRARIES})
                set(draco_FOUND TRUE)
            endif()
        endif()
    endif()
    
    # Fetch Draco if not found
    if(NOT draco_FOUND)
        message(STATUS "Draco not found. Fetching from GitHub...")
        include(FetchContent)
        FetchContent_Declare(
            draco
            GIT_REPOSITORY https://github.com/google/draco.git
            GIT_TAG 1.5.7
            GIT_SHALLOW TRUE
        )
        
        set(DRACO_TRANSCODER_SUPPORTED OFF CACHE BOOL "" FORCE)
        set(DRACO_TESTS OFF CACHE BOOL "" FORCE)
        set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
        
        FetchContent_MakeAvailable(draco)
        set(draco_FOUND TRUE)
    endif()
    
    if(draco_FOUND)
        set(PYLMESH_USE_DRACO TRUE)
        message(STATUS "Draco compression enabled")
    else()
        message(WARNING "Draco requested but not available. GLB will be uncompressed.")
    endif()
endif()
