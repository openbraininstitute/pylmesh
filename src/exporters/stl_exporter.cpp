#include "pylmesh/exporters/stl_exporter.h"
#include <fstream>

namespace pylmesh {

bool STLExporter::canSave(const std::string& filepath) const {
    return filepath.size() >= 4 && filepath.substr(filepath.size() - 4) == ".stl";
}

bool STLExporter::save(const std::string& filepath, const Mesh& mesh) {
    std::ofstream file(filepath);
    if (!file.is_open()) return false;

    file << "solid mesh\n";

    for (const auto& face : mesh.faces) {
        if (face.indices.size() >= 3) {
            file << "  facet normal 0 0 0\n";
            file << "    outer loop\n";
            for (size_t i = 0; i < 3; ++i) {
                const auto& v = mesh.vertices[face.indices[i]];
                file << "      vertex " << v.x << " " << v.y << " " << v.z << "\n";
            }
            file << "    endloop\n";
            file << "  endfacet\n";
        }
    }

    file << "endsolid mesh\n";
    return true;
}

}
