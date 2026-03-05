#pragma once
#include "mesh.h"
#include <memory>

namespace pylmesh {

class MeshLoader {
public:
    virtual ~MeshLoader() = default;
    virtual bool load(const std::string& filepath, Mesh& mesh) = 0;
    virtual bool canLoad(const std::string& filepath) const = 0;
};

class MeshLoaderFactory {
public:
    static std::unique_ptr<MeshLoader> createLoader(const std::string& filepath);
    static bool loadMesh(const std::string& filepath, Mesh& mesh);
};

}
