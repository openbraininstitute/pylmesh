#include "pylmesh/mesh.h"

namespace pylmesh {

void Mesh::clear() {
    vertices.clear();
    normals.clear();
    texcoords.clear();
    faces.clear();
}

bool Mesh::isEmpty() const {
    return vertices.empty();
}

size_t Mesh::vertexCount() const {
    return vertices.size();
}

size_t Mesh::faceCount() const {
    return faces.size();
}

}
