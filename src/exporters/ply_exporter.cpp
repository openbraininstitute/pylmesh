#include "pylmesh/exporters/ply_exporter.h"
#include <fstream>

namespace pylmesh {

bool PLYExporter::canSave(const std::string& filepath) const {
    return filepath.size() >= 4 && filepath.substr(filepath.size() - 4) == ".ply";
}

bool PLYExporter::save(const std::string& filepath, const Mesh& mesh) {
    std::ofstream file(filepath);
    if (!file.is_open()) return false;

    file << "ply\n";
    file << "format ascii 1.0\n";
    file << "element vertex " << mesh.vertices.size() << "\n";
    file << "property float x\n";
    file << "property float y\n";
    file << "property float z\n";
    file << "element face " << mesh.faces.size() << "\n";
    file << "property list uchar int vertex_indices\n";
    file << "end_header\n";

    for (const auto& v : mesh.vertices) {
        file << v.x << " " << v.y << " " << v.z << "\n";
    }

    for (const auto& f : mesh.faces) {
        file << f.indices.size();
        for (auto idx : f.indices) {
            file << " " << idx;
        }
        file << "\n";
    }

    return true;
}

}
