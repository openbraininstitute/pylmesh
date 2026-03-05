#pragma once
#include "mesh.h"
#include <memory>

namespace pylmesh {

class MeshExporter {
public:
    virtual ~MeshExporter() = default;
    virtual bool save(const std::string& filepath, const Mesh& mesh) = 0;
    virtual bool canSave(const std::string& filepath) const = 0;
};

class MeshExporterFactory {
public:
    static std::unique_ptr<MeshExporter> createExporter(const std::string& filepath);
    static bool saveMesh(const std::string& filepath, const Mesh& mesh);
};

}
