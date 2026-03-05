#include "pylmesh/exporters/off_exporter.h"
#include <fstream>

namespace pylmesh {

bool OFFExporter::canSave(const std::string& filepath) const {
    return filepath.size() >= 4 && filepath.substr(filepath.size() - 4) == ".off";
}

bool OFFExporter::save(const std::string& filepath, const Mesh& mesh) {
    std::ofstream file(filepath);
    if (!file.is_open()) return false;

    file << "OFF\n";
    file << mesh.vertices.size() << " " << mesh.faces.size() << " 0\n";

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
