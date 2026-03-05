#include "pylmesh/loaders/ply_loader.h"
#include <fstream>
#include <sstream>

namespace pylmesh {

bool PLYLoader::canLoad(const std::string& filepath) const {
    return filepath.size() >= 4 && filepath.substr(filepath.size() - 4) == ".ply";
}

bool PLYLoader::load(const std::string& filepath, Mesh& mesh) {
    std::ifstream file(filepath);
    if (!file.is_open()) return false;

    mesh.clear();
    std::string line;
    int vertexCount = 0, faceCount = 0;
    bool headerDone = false;

    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string keyword;
        iss >> keyword;

        if (keyword == "element") {
            std::string type;
            int count;
            iss >> type >> count;
            if (type == "vertex") vertexCount = count;
            if (type == "face") faceCount = count;
        } else if (keyword == "end_header") {
            headerDone = true;
            break;
        }
    }

    if (headerDone) {
        for (int i = 0; i < vertexCount; ++i) {
            Vertex v;
            file >> v.x >> v.y >> v.z;
            mesh.vertices.push_back(v);
            std::getline(file, line);
        }

        for (int i = 0; i < faceCount; ++i) {
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
    }

    return !mesh.isEmpty();
}

}
