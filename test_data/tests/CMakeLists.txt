add_executable(test_mesh test_mesh.cpp)
target_link_libraries(test_mesh PRIVATE pylmesh)
add_test(NAME MeshTest COMMAND test_mesh)
