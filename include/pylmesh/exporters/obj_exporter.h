#pragma once
#include "../exporter.h"

namespace pylmesh {

class OBJExporter : public MeshExporter {
public:
    bool save(const std::string& filepath, const Mesh& mesh) override;
    bool canSave(const std::string& filepath) const override;
};

}
