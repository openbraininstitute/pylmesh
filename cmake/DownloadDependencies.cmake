# Download external dependencies for GLTF/GLB support

if(ENABLE_GLTF)
    set(EXTERNAL_DIR "${CMAKE_SOURCE_DIR}/include/pylmesh/external")
    file(MAKE_DIRECTORY ${EXTERNAL_DIR})
    
    # Download tinygltf
    if(NOT EXISTS "${EXTERNAL_DIR}/tiny_gltf.h")
        message(STATUS "Downloading tinygltf...")
        file(DOWNLOAD
            "https://raw.githubusercontent.com/syoyo/tinygltf/release/tiny_gltf.h"
            "${EXTERNAL_DIR}/tiny_gltf.h"
            SHOW_PROGRESS)
    endif()
    
    # Download nlohmann json
    if(NOT EXISTS "${EXTERNAL_DIR}/json.hpp")
        message(STATUS "Downloading nlohmann/json...")
        file(DOWNLOAD
            "https://raw.githubusercontent.com/nlohmann/json/develop/single_include/nlohmann/json.hpp"
            "${EXTERNAL_DIR}/json.hpp"
            SHOW_PROGRESS)
    endif()
    
    # Download stb_image
    if(NOT EXISTS "${EXTERNAL_DIR}/stb_image.h")
        message(STATUS "Downloading stb_image...")
        file(DOWNLOAD
            "https://raw.githubusercontent.com/nothings/stb/master/stb_image.h"
            "${EXTERNAL_DIR}/stb_image.h"
            SHOW_PROGRESS)
    endif()
    
    # Download stb_image_write
    if(NOT EXISTS "${EXTERNAL_DIR}/stb_image_write.h")
        message(STATUS "Downloading stb_image_write...")
        file(DOWNLOAD
            "https://raw.githubusercontent.com/nothings/stb/master/stb_image_write.h"
            "${EXTERNAL_DIR}/stb_image_write.h"
            SHOW_PROGRESS)
    endif()
endif()
