#include "pylmesh/loaders/off_loader.h"
#include <fstream>
#include <sstream>

namespace pylmesh {

bool OFFLoader::canLoad(const std::string& filepath) const {
    return filepath.size() >= 4 && filepath.substr(filepath.size() - 4) == ".off";
}

bool OFFLoader::load(const std::string& filepath, Mesh& mesh) {
    std::ifstream file(filepath);
    if (!file.is_open()) return false;

    mesh.clear();
    std::string header;
    file >> header;
    if (header != "OFF") return false;

    int nVertices, nFaces, nEdges;
    file >> nVertices >> nFaces >> nEdges;

    for (int i = 0; i < nVertices; ++i) {
        Vertex v;
        file >> v.x >> v.y >> v.z;
        mesh.vertices.push_back(v);
    }

    for (int i = 0; i < nFaces; ++i) {
        int n;
        file >> n;
        Face f;
        for (int j = 0; j < n; ++j) {
            unsigned int idx;
            file >> idx;
            f.indices.push_back(idx);
        }
        mesh.faces.push_back(f);
    }

    return !mesh.isEmpty();
}

}
