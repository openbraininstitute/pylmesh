#pragma once
#include "../loader.h"

namespace pylmesh {

class PLYLoader : public MeshLoader {
public:
    bool load(const std::string& filepath, Mesh& mesh) override;
    bool canLoad(const std::string& filepath) const override;
};

}
