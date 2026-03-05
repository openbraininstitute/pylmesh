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

std::vector<float> Mesh::getVerticesArray() const {
    std::vector<float> result;
    result.reserve(vertices.size() * 3);
    for (const auto& v : vertices) {
        result.push_back(v.x);
        result.push_back(v.y);
        result.push_back(v.z);
    }
    return result;
}

std::vector<unsigned int> Mesh::getFacesArray() const {
    std::vector<unsigned int> result;
    for (const auto& f : faces) {
        for (auto idx : f.indices) {
            result.push_back(idx);
        }
    }
    return result;
}

}
