#include "pylmesh/loaders/stl_loader.h"
#include <fstream>
#include <sstream>

namespace pylmesh {

bool STLLoader::canLoad(const std::string& filepath) const {
    return filepath.size() >= 4 && filepath.substr(filepath.size() - 4) == ".stl";
}

bool STLLoader::load(const std::string& filepath, Mesh& mesh) {
    std::ifstream file(filepath);
    if (!file.is_open()) return false;

    mesh.clear();
    std::string line;
    
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string keyword;
        iss >> keyword;

        if (keyword == "vertex") {
            Vertex v;
            iss >> v.x >> v.y >> v.z;
            mesh.vertices.push_back(v);
        }
    }

    for (size_t i = 0; i < mesh.vertices.size(); i += 3) {
        Face f;
        f.indices = {(unsigned int)i, (unsigned int)i+1, (unsigned int)i+2};
        mesh.faces.push_back(f);
    }

    return !mesh.isEmpty();
}

}
