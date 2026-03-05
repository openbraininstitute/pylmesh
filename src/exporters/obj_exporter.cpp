#include "pylmesh/exporters/obj_exporter.h"
#include <fstream>

namespace pylmesh {

bool OBJExporter::canSave(const std::string& filepath) const {
    return filepath.size() >= 4 && filepath.substr(filepath.size() - 4) == ".obj";
}

bool OBJExporter::save(const std::string& filepath, const Mesh& mesh) {
    std::ofstream file(filepath);
    if (!file.is_open()) return false;

    file << "# OBJ file exported by pylmesh\n";

    for (const auto& v : mesh.vertices) {
        file << "v " << v.x << " " << v.y << " " << v.z << "\n";
    }

    for (const auto& n : mesh.normals) {
        file << "vn " << n.nx << " " << n.ny << " " << n.nz << "\n";
    }

    for (const auto& t : mesh.texcoords) {
        file << "vt " << t.u << " " << t.v << "\n";
    }

    for (const auto& f : mesh.faces) {
        file << "f";
        for (auto idx : f.indices) {
            file << " " << (idx + 1);
        }
        file << "\n";
    }

    return true;
}

}
