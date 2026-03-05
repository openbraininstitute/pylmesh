#pragma once
#include "../exporter.h"

namespace pylmesh {

class GLTFExporter : public MeshExporter {
public:
    bool save(const std::string& filepath, const Mesh& mesh) override;
    bool canSave(const std::string& filepath) const override;
};

class GLBExporter : public MeshExporter {
public:
    bool save(const std::string& filepath, const Mesh& mesh) override;
    bool canSave(const std::string& filepath) const override;
};

}
