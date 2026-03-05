# Setup Python bindings with pybind11

if(BUILD_PYTHON)
    find_package(pybind11 CONFIG QUIET)
    if(pybind11_FOUND)
        pybind11_add_module(_pylmesh python/bindings.cpp)
        target_link_libraries(_pylmesh PRIVATE pylmesh)
        install(TARGETS _pylmesh LIBRARY DESTINATION . COMPONENT python)
        install(FILES python/pylmesh/__init__.py DESTINATION . COMPONENT python)
    endif()
endif()
