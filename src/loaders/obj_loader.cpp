#include "pylmesh/loaders/obj_loader.h"
#include <fstream>
#include <sstream>

namespace pylmesh {

bool OBJLoader::canLoad(const std::string& filepath) const {
    return filepath.size() >= 4 && filepath.substr(filepath.size() - 4) == ".obj";
}

bool OBJLoader::load(const std::string& filepath, Mesh& mesh) {
    std::ifstream file(filepath);
    if (!file.is_open()) return false;

    mesh.clear();
    std::string line;
    
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string prefix;
        iss >> prefix;

        if (prefix == "v") {
            Vertex v;
            iss >> v.x >> v.y >> v.z;
            mesh.vertices.push_back(v);
        } else if (prefix == "vn") {
            Normal n;
            iss >> n.nx >> n.ny >> n.nz;
            mesh.normals.push_back(n);
        } else if (prefix == "vt") {
            TexCoord t;
            iss >> t.u >> t.v;
            mesh.texcoords.push_back(t);
        } else if (prefix == "f") {
            Face f;
            std::string vertex;
            while (iss >> vertex) {
                unsigned int idx = std::stoi(vertex.substr(0, vertex.find('/'))) - 1;
                f.indices.push_back(idx);
            }
            mesh.faces.push_back(f);
        }
    }

    return !mesh.isEmpty();
}

}
