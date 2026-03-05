#include "pylmesh/exporters/gltf_exporter.h"
#include <cstring>
#include <algorithm>
#include <limits>

#ifdef PYLMESH_USE_TINYGLTF
#include "tiny_gltf.h"
#endif

namespace pylmesh {

bool GLTFExporter::canSave(const std::string& filepath) const {
    return filepath.size() >= 5 && filepath.substr(filepath.size() - 5) == ".gltf";
}

bool GLTFExporter::save(const std::string& filepath, const Mesh& mesh) {
#ifdef PYLMESH_USE_TINYGLTF
    tinygltf::Model model;
    tinygltf::Scene scene;
    tinygltf::Mesh gltfMesh;
    tinygltf::Primitive primitive;
    tinygltf::Buffer buffer;

    // Pack positions
    std::vector<float> positions;
    positions.reserve(mesh.vertices.size() * 3);
    float pMin[3] = {std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max()};
    float pMax[3] = {-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max(), -std::numeric_limits<float>::max()};

    for (const auto& v : mesh.vertices) {
        positions.push_back(v.x);
        positions.push_back(v.y);
        positions.push_back(v.z);
        pMin[0] = std::min(pMin[0], v.x);
        pMin[1] = std::min(pMin[1], v.y);
        pMin[2] = std::min(pMin[2], v.z);
        pMax[0] = std::max(pMax[0], v.x);
        pMax[1] = std::max(pMax[1], v.y);
        pMax[2] = std::max(pMax[2], v.z);
    }

    // Pack indices
    std::vector<uint32_t> indices;
    for (const auto& f : mesh.faces) {
        for (auto idx : f.indices) {
            indices.push_back(idx);
        }
    }

    // Create buffer
    size_t posSize = positions.size() * sizeof(float);
    size_t idxSize = indices.size() * sizeof(uint32_t);
    buffer.data.resize(posSize + idxSize);
    std::memcpy(&buffer.data[0], positions.data(), posSize);
    std::memcpy(&buffer.data[posSize], indices.data(), idxSize);

    // Position buffer view
    tinygltf::BufferView positionView;
    positionView.buffer = 0;
    positionView.byteOffset = 0;
    positionView.byteLength = posSize;
    positionView.target = TINYGLTF_TARGET_ARRAY_BUFFER;

    // Index buffer view
    tinygltf::BufferView indexView;
    indexView.buffer = 0;
    indexView.byteOffset = posSize;
    indexView.byteLength = idxSize;
    indexView.target = TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER;

    // Position accessor
    tinygltf::Accessor positionAccessor;
    positionAccessor.bufferView = 0;
    positionAccessor.byteOffset = 0;
    positionAccessor.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
    positionAccessor.count = mesh.vertices.size();
    positionAccessor.type = TINYGLTF_TYPE_VEC3;
    positionAccessor.minValues = {pMin[0], pMin[1], pMin[2]};
    positionAccessor.maxValues = {pMax[0], pMax[1], pMax[2]};

    // Index accessor
    tinygltf::Accessor indexAccessor;
    indexAccessor.bufferView = 1;
    indexAccessor.byteOffset = 0;
    indexAccessor.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT;
    indexAccessor.count = indices.size();
    indexAccessor.type = TINYGLTF_TYPE_SCALAR;

    // Build primitive
    primitive.attributes["POSITION"] = 0;
    primitive.indices = 1;
    primitive.mode = TINYGLTF_MODE_TRIANGLES;

    // Assemble model
    gltfMesh.primitives.push_back(primitive);
    model.meshes.push_back(gltfMesh);
    model.buffers.push_back(buffer);
    model.bufferViews.push_back(positionView);
    model.bufferViews.push_back(indexView);
    model.accessors.push_back(positionAccessor);
    model.accessors.push_back(indexAccessor);

    tinygltf::Node node;
    node.mesh = 0;
    model.nodes.push_back(node);

    scene.nodes.push_back(0);
    model.scenes.push_back(scene);
    model.defaultScene = 0;

    tinygltf::TinyGLTF writer;
    return writer.WriteGltfSceneToFile(&model, filepath, false, true, true, false);
#else
    return false;
#endif
}

bool GLBExporter::canSave(const std::string& filepath) const {
    return filepath.size() >= 4 && filepath.substr(filepath.size() - 4) == ".glb";
}

bool GLBExporter::save(const std::string& filepath, const Mesh& mesh) {
#ifdef PYLMESH_USE_TINYGLTF
    tinygltf::Model model;
    tinygltf::Scene scene;
    tinygltf::Mesh gltfMesh;
    tinygltf::Primitive primitive;
    tinygltf::Buffer buffer;

    // Pack positions
    std::vector<float> positions;
    positions.reserve(mesh.vertices.size() * 3);
    float pMin[3] = {std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max()};
    float pMax[3] = {-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max(), -std::numeric_limits<float>::max()};

    for (const auto& v : mesh.vertices) {
        positions.push_back(v.x);
        positions.push_back(v.y);
        positions.push_back(v.z);
        pMin[0] = std::min(pMin[0], v.x);
        pMin[1] = std::min(pMin[1], v.y);
        pMin[2] = std::min(pMin[2], v.z);
        pMax[0] = std::max(pMax[0], v.x);
        pMax[1] = std::max(pMax[1], v.y);
        pMax[2] = std::max(pMax[2], v.z);
    }

    // Pack indices
    std::vector<uint32_t> indices;
    for (const auto& f : mesh.faces) {
        for (auto idx : f.indices) {
            indices.push_back(idx);
        }
    }

    // Create buffer
    size_t posSize = positions.size() * sizeof(float);
    size_t idxSize = indices.size() * sizeof(uint32_t);
    buffer.data.resize(posSize + idxSize);
    std::memcpy(&buffer.data[0], positions.data(), posSize);
    std::memcpy(&buffer.data[posSize], indices.data(), idxSize);

    // Position buffer view
    tinygltf::BufferView positionView;
    positionView.buffer = 0;
    positionView.byteOffset = 0;
    positionView.byteLength = posSize;
    positionView.target = TINYGLTF_TARGET_ARRAY_BUFFER;

    // Index buffer view
    tinygltf::BufferView indexView;
    indexView.buffer = 0;
    indexView.byteOffset = posSize;
    indexView.byteLength = idxSize;
    indexView.target = TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER;

    // Position accessor
    tinygltf::Accessor positionAccessor;
    positionAccessor.bufferView = 0;
    positionAccessor.byteOffset = 0;
    positionAccessor.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
    positionAccessor.count = mesh.vertices.size();
    positionAccessor.type = TINYGLTF_TYPE_VEC3;
    positionAccessor.minValues = {pMin[0], pMin[1], pMin[2]};
    positionAccessor.maxValues = {pMax[0], pMax[1], pMax[2]};

    // Index accessor
    tinygltf::Accessor indexAccessor;
    indexAccessor.bufferView = 1;
    indexAccessor.byteOffset = 0;
    indexAccessor.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT;
    indexAccessor.count = indices.size();
    indexAccessor.type = TINYGLTF_TYPE_SCALAR;

    // Build primitive
    primitive.attributes["POSITION"] = 0;
    primitive.indices = 1;
    primitive.mode = TINYGLTF_MODE_TRIANGLES;

    // Assemble model
    gltfMesh.primitives.push_back(primitive);
    model.meshes.push_back(gltfMesh);
    model.buffers.push_back(buffer);
    model.bufferViews.push_back(positionView);
    model.bufferViews.push_back(indexView);
    model.accessors.push_back(positionAccessor);
    model.accessors.push_back(indexAccessor);

    tinygltf::Node node;
    node.mesh = 0;
    model.nodes.push_back(node);

    scene.nodes.push_back(0);
    model.scenes.push_back(scene);
    model.defaultScene = 0;

    tinygltf::TinyGLTF writer;
    return writer.WriteGltfSceneToFile(&model, filepath, false, true, true, true);
#else
    return false;
#endif
}

}
