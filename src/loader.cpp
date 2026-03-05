#include "pylmesh/loader.h"
#include "pylmesh/loaders/obj_loader.h"
#include "pylmesh/loaders/stl_loader.h"
#include "pylmesh/loaders/ply_loader.h"

namespace pylmesh {

std::unique_ptr<MeshLoader> MeshLoaderFactory::createLoader(const std::string& filepath) {
    if (OBJLoader().canLoad(filepath)) {
        return std::make_unique<OBJLoader>();
    }
    if (STLLoader().canLoad(filepath)) {
        return std::make_unique<STLLoader>();
    }
    if (PLYLoader().canLoad(filepath)) {
        return std::make_unique<PLYLoader>();
    }
    return nullptr;
}

bool MeshLoaderFactory::loadMesh(const std::string& filepath, Mesh& mesh) {
    auto loader = createLoader(filepath);
    if (!loader) return false;
    return loader->load(filepath, mesh);
}

}
