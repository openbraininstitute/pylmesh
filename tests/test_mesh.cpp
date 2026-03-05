#include <pylmesh/mesh.h>
#include <iostream>

int main() {
    pylmesh::Mesh mesh;
    
    if (!mesh.isEmpty()) {
        std::cerr << "New mesh should be empty\n";
        return 1;
    }

    mesh.vertices.push_back({0, 0, 0});
    mesh.vertices.push_back({1, 0, 0});
    mesh.vertices.push_back({0, 1, 0});

    if (mesh.vertexCount() != 3) {
        std::cerr << "Expected 3 vertices\n";
        return 1;
    }

    mesh.clear();
    if (!mesh.isEmpty()) {
        std::cerr << "Mesh should be empty after clear\n";
        return 1;
    }

    std::cout << "All tests passed\n";
    return 0;
}
