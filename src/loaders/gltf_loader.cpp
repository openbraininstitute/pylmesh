#include "pylmesh/loaders/gltf_loader.h"

#ifdef PYLMESH_USE_TINYGLTF
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "tiny_gltf.h"
#endif

namespace pylmesh {

bool GLTFLoader::canLoad(const std::string& filepath) const {
    size_t len = filepath.size();
    return (len >= 5 && filepath.substr(len - 5) == ".gltf") ||
           (len >= 4 && filepath.substr(len - 4) == ".glb");
}

bool GLTFLoader::load(const std::string& filepath, Mesh& mesh) {
#ifdef PYLMESH_USE_TINYGLTF
    tinygltf::Model model;
    tinygltf::TinyGLTF loader;
    std::string err, warn;

    bool ret;
    if (filepath.substr(filepath.size() - 4) == ".glb") {
        ret = loader.LoadBinaryFromFile(&model, &err, &warn, filepath);
    } else {
        ret = loader.LoadASCIIFromFile(&model, &err, &warn, filepath);
    }

    if (!ret) return false;

    mesh.clear();

    for (const auto& gltfMesh : model.meshes) {
        for (const auto& primitive : gltfMesh.primitives) {
            if (primitive.mode != TINYGLTF_MODE_TRIANGLES) continue;

            size_t vertexOffset = mesh.vertices.size();

            // Load positions
            auto posIt = primitive.attributes.find("POSITION");
            if (posIt != primitive.attributes.end()) {
                const auto& accessor = model.accessors[posIt->second];
                const auto& bufferView = model.bufferViews[accessor.bufferView];
                const auto& buffer = model.buffers[bufferView.buffer];
                const float* positions = reinterpret_cast<const float*>(
                    &buffer.data[bufferView.byteOffset + accessor.byteOffset]);

                for (size_t i = 0; i < accessor.count; ++i) {
                    Vertex v;
                    v.x = positions[i * 3];
                    v.y = positions[i * 3 + 1];
                    v.z = positions[i * 3 + 2];
                    mesh.vertices.push_back(v);
                }
            }

            // Load indices
            if (primitive.indices >= 0) {
                const auto& accessor = model.accessors[primitive.indices];
                const auto& bufferView = model.bufferViews[accessor.bufferView];
                const auto& buffer = model.buffers[bufferView.buffer];
                const uint8_t* data = &buffer.data[bufferView.byteOffset + accessor.byteOffset];

                for (size_t i = 0; i < accessor.count; i += 3) {
                    Face f;
                    for (int j = 0; j < 3; ++j) {
                        uint32_t idx;
                        if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
                            idx = reinterpret_cast<const uint16_t*>(data)[i + j];
                        else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT)
                            idx = reinterpret_cast<const uint32_t*>(data)[i + j];
                        else
                            idx = data[i + j];
                        f.indices.push_back(vertexOffset + idx);
                    }
                    mesh.faces.push_back(f);
                }
            }
        }
    }

    return !mesh.isEmpty();
#else
    return false;
#endif
}

}
