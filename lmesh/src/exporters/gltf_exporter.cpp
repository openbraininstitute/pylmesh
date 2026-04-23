/*****************************************************************************************
 * Copyright (c) 2025 - 2026, Open Brain Institute
 *
 * Author(s):
 *   Marwan Abdellah <marwan.abdellah@openbraininstitute.org>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under
 * the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *****************************************************************************************/

#include "lmesh/exporters/gltf_exporter.h"
#include "lmesh/quantized_mesh.h"
#include <algorithm>
#include <cstring>
#include <limits>

#ifdef PYLMESH_USE_TINYGLTF
#include "tiny_gltf.h"
#endif

#ifdef PYLMESH_USE_DRACO
#include "draco/compression/encode.h"
#include "draco/mesh/mesh.h"
#endif

namespace pylmesh
{

bool GLTFExporter::canSave(const std::string& filepath) const
{
    return filepath.size() >= 5 && filepath.substr(filepath.size() - 5) == ".gltf";
}

bool GLTFExporter::save(const std::string& filepath, const Mesh& mesh)
{
#ifdef PYLMESH_USE_TINYGLTF
    tinygltf::Model model;
    tinygltf::Scene scene;
    tinygltf::Mesh gltfMesh;
    tinygltf::Primitive primitive;
    tinygltf::Buffer buffer;

    // Pack positions
    std::vector<float> positions;
    positions.reserve(mesh.vertices.size() * 3);
    float pMin[3] = {std::numeric_limits<float>::max(), std::numeric_limits<float>::max(),
                     std::numeric_limits<float>::max()};
    float pMax[3] = {-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max(),
                     -std::numeric_limits<float>::max()};

    for (const auto& v : mesh.vertices)
    {
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

    // Indices are already flat
    const auto& indices = mesh.indices;

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

bool GLBExporter::canSave(const std::string& filepath) const
{
    return filepath.size() >= 4 && filepath.substr(filepath.size() - 4) == ".glb";
}

bool GLTFExporter::save(const std::string& filepath, const QuantizedMesh& mesh)
{
#ifdef PYLMESH_USE_TINYGLTF
    const uint32_t nVerts = mesh.vertex_count();
    const uint32_t nFaces = mesh.face_count();

    // Dequantize all vertices and compute bounds
    std::vector<float> positions(nVerts * 3);
    float pMin[3] = { std::numeric_limits<float>::max(),  std::numeric_limits<float>::max(),  std::numeric_limits<float>::max() };
    float pMax[3] = {-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max(), -std::numeric_limits<float>::max() };

    for (uint32_t i = 0; i < nVerts; ++i)
    {
        Vertex v = mesh.get_vertex(i);
        positions[i * 3]     = v.x;
        positions[i * 3 + 1] = v.y;
        positions[i * 3 + 2] = v.z;
        pMin[0] = std::min(pMin[0], v.x); pMax[0] = std::max(pMax[0], v.x);
        pMin[1] = std::min(pMin[1], v.y); pMax[1] = std::max(pMax[1], v.y);
        pMin[2] = std::min(pMin[2], v.z); pMax[2] = std::max(pMax[2], v.z);
    }

    // Decode all face indices
    std::vector<uint32_t> indices(nFaces * 3);
    for (uint32_t i = 0; i < nFaces; ++i)
    {
        auto f = mesh.get_face(i);
        indices[i * 3]     = f[0];
        indices[i * 3 + 1] = f[1];
        indices[i * 3 + 2] = f[2];
    }

    tinygltf::Model model;
    tinygltf::Scene scene;
    tinygltf::Mesh gltfMesh;
    tinygltf::Primitive primitive;
    tinygltf::Buffer buffer;

    size_t posSize = positions.size() * sizeof(float);
    size_t idxSize = indices.size() * sizeof(uint32_t);
    buffer.data.resize(posSize + idxSize);
    std::memcpy(&buffer.data[0], positions.data(), posSize);
    std::memcpy(&buffer.data[posSize], indices.data(), idxSize);

    tinygltf::BufferView posView;
    posView.buffer = 0; posView.byteOffset = 0; posView.byteLength = posSize;
    posView.target = TINYGLTF_TARGET_ARRAY_BUFFER;

    tinygltf::BufferView idxView;
    idxView.buffer = 0; idxView.byteOffset = posSize; idxView.byteLength = idxSize;
    idxView.target = TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER;

    tinygltf::Accessor posAcc;
    posAcc.bufferView = 0; posAcc.byteOffset = 0;
    posAcc.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
    posAcc.count = nVerts; posAcc.type = TINYGLTF_TYPE_VEC3;
    posAcc.minValues = {pMin[0], pMin[1], pMin[2]};
    posAcc.maxValues = {pMax[0], pMax[1], pMax[2]};

    tinygltf::Accessor idxAcc;
    idxAcc.bufferView = 1; idxAcc.byteOffset = 0;
    idxAcc.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT;
    idxAcc.count = indices.size(); idxAcc.type = TINYGLTF_TYPE_SCALAR;

    primitive.attributes["POSITION"] = 0;
    primitive.indices = 1;
    primitive.mode = TINYGLTF_MODE_TRIANGLES;

    gltfMesh.primitives.push_back(primitive);
    model.meshes.push_back(gltfMesh);
    model.buffers.push_back(buffer);
    model.bufferViews.push_back(posView);
    model.bufferViews.push_back(idxView);
    model.accessors.push_back(posAcc);
    model.accessors.push_back(idxAcc);

    tinygltf::Node node; node.mesh = 0;
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

bool GLBExporter::save(const std::string& filepath, const Mesh& mesh)
{
#if defined(PYLMESH_USE_TINYGLTF) && defined(PYLMESH_USE_DRACO)
    // Use Draco compression
    tinygltf::Model model;
    tinygltf::Scene scene;
    tinygltf::Mesh gltfMesh;
    tinygltf::Primitive primitive;

    // Create Draco mesh
    draco::Mesh dracoMesh;
    dracoMesh.set_num_points(mesh.vertices.size());
    dracoMesh.SetNumFaces(mesh.faceCount());

    // Add position attribute
    draco::GeometryAttribute posAttr;
    posAttr.Init(draco::GeometryAttribute::POSITION, nullptr, 3, draco::DT_FLOAT32, false,
                 sizeof(float) * 3, 0);
    int posAttId = dracoMesh.AddAttribute(posAttr, true, mesh.vertices.size());

    float pMin[3] = {std::numeric_limits<float>::max(), std::numeric_limits<float>::max(),
                     std::numeric_limits<float>::max()};
    float pMax[3] = {-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max(),
                     -std::numeric_limits<float>::max()};

    // Set vertex positions
    for (size_t i = 0; i < mesh.vertices.size(); ++i)
    {
        float pos[3] = {mesh.vertices[i].x, mesh.vertices[i].y, mesh.vertices[i].z};
        dracoMesh.attribute(posAttId)->SetAttributeValue(draco::AttributeValueIndex(i), pos);
        pMin[0] = std::min(pMin[0], pos[0]);
        pMin[1] = std::min(pMin[1], pos[1]);
        pMin[2] = std::min(pMin[2], pos[2]);
        pMax[0] = std::max(pMax[0], pos[0]);
        pMax[1] = std::max(pMax[1], pos[1]);
        pMax[2] = std::max(pMax[2], pos[2]);
    }

    // Set faces
    for (size_t i = 0; i < mesh.faceCount(); ++i)
    {
        const uint32_t* idx = mesh.faceIndices(i);
        if (mesh.faceSize(i) >= 3)
        {
            draco::Mesh::Face face;
            face[0] = idx[0];
            face[1] = idx[1];
            face[2] = idx[2];
            dracoMesh.SetFace(draco::FaceIndex(i), face);
        }
    }

    // Encode with Draco
    draco::Encoder encoder;
    encoder.SetSpeedOptions(5, 5);
    encoder.SetAttributeQuantization(draco::GeometryAttribute::POSITION, 14);

    draco::EncoderBuffer encBuffer;
    if (!encoder.EncodeMeshToBuffer(dracoMesh, &encBuffer).ok())
    {
        return false;
    }

    // Create buffer with Draco data
    tinygltf::Buffer buffer;
    buffer.data.assign(encBuffer.data(), encBuffer.data() + encBuffer.size());

    tinygltf::BufferView bufferView;
    bufferView.buffer = 0;
    bufferView.byteOffset = 0;
    bufferView.byteLength = buffer.data.size();

    tinygltf::Accessor posAccessor;
    posAccessor.bufferView = 0;
    posAccessor.byteOffset = 0;
    posAccessor.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
    posAccessor.count = mesh.vertices.size();
    posAccessor.type = TINYGLTF_TYPE_VEC3;
    posAccessor.minValues = {pMin[0], pMin[1], pMin[2]};
    posAccessor.maxValues = {pMax[0], pMax[1], pMax[2]};

    primitive.attributes["POSITION"] = 0;
    primitive.mode = TINYGLTF_MODE_TRIANGLES;

    // Add Draco extension
    tinygltf::Value::Object dracoObj;
    dracoObj["bufferView"] = tinygltf::Value(0);
    tinygltf::Value::Object attrsObj;
    attrsObj["POSITION"] = tinygltf::Value(posAttId);
    dracoObj["attributes"] = tinygltf::Value(attrsObj);
    primitive.extensions["KHR_draco_mesh_compression"] = tinygltf::Value(dracoObj);

    // Placeholder index accessor
    tinygltf::Accessor idxAccessor;
    idxAccessor.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT;
    idxAccessor.count = mesh.indices.size();
    idxAccessor.type = TINYGLTF_TYPE_SCALAR;
    idxAccessor.bufferView = -1;
    primitive.indices = 1;

    model.buffers.push_back(buffer);
    model.bufferViews.push_back(bufferView);
    model.accessors.push_back(posAccessor);
    model.accessors.push_back(idxAccessor);

    model.extensionsUsed.push_back("KHR_draco_mesh_compression");
    model.extensionsRequired.push_back("KHR_draco_mesh_compression");

    gltfMesh.primitives.push_back(primitive);
    model.meshes.push_back(gltfMesh);

    tinygltf::Node node;
    node.mesh = 0;
    model.nodes.push_back(node);

    scene.nodes.push_back(0);
    model.scenes.push_back(scene);
    model.defaultScene = 0;

    tinygltf::TinyGLTF writer;
    return writer.WriteGltfSceneToFile(&model, filepath, false, true, true, true);
#elif defined(PYLMESH_USE_TINYGLTF)
    // Fallback to uncompressed GLB
    tinygltf::Model model;
    tinygltf::Scene scene;
    tinygltf::Mesh gltfMesh;
    tinygltf::Primitive primitive;
    tinygltf::Buffer buffer;

    // Pack positions
    std::vector<float> positions;
    positions.reserve(mesh.vertices.size() * 3);
    float pMin[3] = {std::numeric_limits<float>::max(), std::numeric_limits<float>::max(),
                     std::numeric_limits<float>::max()};
    float pMax[3] = {-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max(),
                     -std::numeric_limits<float>::max()};

    for (const auto& v : mesh.vertices)
    {
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

    // Indices are already flat
    const auto& indices = mesh.indices;

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

bool GLBExporter::save(const std::string& filepath, const QuantizedMesh& mesh)
{
#if defined(PYLMESH_USE_TINYGLTF) && defined(PYLMESH_USE_DRACO)
    const uint32_t nVerts = mesh.vertex_count();
    const uint32_t nFaces = mesh.face_count();

    // Dequantize vertices and compute bounds
    float pMin[3] = { std::numeric_limits<float>::max(),  std::numeric_limits<float>::max(),  std::numeric_limits<float>::max() };
    float pMax[3] = {-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max(), -std::numeric_limits<float>::max() };

    draco::Mesh dracoMesh;
    dracoMesh.set_num_points(nVerts);
    dracoMesh.SetNumFaces(nFaces);

    draco::GeometryAttribute posAttr;
    posAttr.Init(draco::GeometryAttribute::POSITION, nullptr, 3, draco::DT_FLOAT32, false,
                 sizeof(float) * 3, 0);
    int posAttId = dracoMesh.AddAttribute(posAttr, true, nVerts);

    for (uint32_t i = 0; i < nVerts; ++i)
    {
        Vertex v = mesh.get_vertex(i);
        float pos[3] = {v.x, v.y, v.z};
        dracoMesh.attribute(posAttId)->SetAttributeValue(draco::AttributeValueIndex(i), pos);
        pMin[0] = std::min(pMin[0], v.x); pMax[0] = std::max(pMax[0], v.x);
        pMin[1] = std::min(pMin[1], v.y); pMax[1] = std::max(pMax[1], v.y);
        pMin[2] = std::min(pMin[2], v.z); pMax[2] = std::max(pMax[2], v.z);
    }

    for (uint32_t i = 0; i < nFaces; ++i)
    {
        auto f = mesh.get_face(i);
        draco::Mesh::Face face;
        face[0] = f[0]; face[1] = f[1]; face[2] = f[2];
        dracoMesh.SetFace(draco::FaceIndex(i), face);
    }

    draco::Encoder encoder;
    encoder.SetSpeedOptions(5, 5);
    encoder.SetAttributeQuantization(draco::GeometryAttribute::POSITION, 14);

    draco::EncoderBuffer encBuffer;
    if (!encoder.EncodeMeshToBuffer(dracoMesh, &encBuffer).ok())
        return false;

    tinygltf::Model model;
    tinygltf::Scene scene;
    tinygltf::Mesh gltfMesh;
    tinygltf::Primitive primitive;

    tinygltf::Buffer buffer;
    buffer.data.assign(encBuffer.data(), encBuffer.data() + encBuffer.size());

    tinygltf::BufferView bufferView;
    bufferView.buffer = 0; bufferView.byteOffset = 0;
    bufferView.byteLength = buffer.data.size();

    tinygltf::Accessor posAcc;
    posAcc.bufferView = 0; posAcc.byteOffset = 0;
    posAcc.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
    posAcc.count = nVerts; posAcc.type = TINYGLTF_TYPE_VEC3;
    posAcc.minValues = {pMin[0], pMin[1], pMin[2]};
    posAcc.maxValues = {pMax[0], pMax[1], pMax[2]};

    primitive.attributes["POSITION"] = 0;
    primitive.mode = TINYGLTF_MODE_TRIANGLES;

    tinygltf::Value::Object dracoObj;
    dracoObj["bufferView"] = tinygltf::Value(0);
    tinygltf::Value::Object attrsObj;
    attrsObj["POSITION"] = tinygltf::Value(posAttId);
    dracoObj["attributes"] = tinygltf::Value(attrsObj);
    primitive.extensions["KHR_draco_mesh_compression"] = tinygltf::Value(dracoObj);

    tinygltf::Accessor idxAcc;
    idxAcc.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT;
    idxAcc.count = nFaces * 3;
    idxAcc.type = TINYGLTF_TYPE_SCALAR;
    idxAcc.bufferView = -1;
    primitive.indices = 1;

    model.buffers.push_back(buffer);
    model.bufferViews.push_back(bufferView);
    model.accessors.push_back(posAcc);
    model.accessors.push_back(idxAcc);
    model.extensionsUsed.push_back("KHR_draco_mesh_compression");
    model.extensionsRequired.push_back("KHR_draco_mesh_compression");

    gltfMesh.primitives.push_back(primitive);
    model.meshes.push_back(gltfMesh);

    tinygltf::Node node; node.mesh = 0;
    model.nodes.push_back(node);
    scene.nodes.push_back(0);
    model.scenes.push_back(scene);
    model.defaultScene = 0;

    tinygltf::TinyGLTF writer;
    return writer.WriteGltfSceneToFile(&model, filepath, false, true, true, true);
#elif defined(PYLMESH_USE_TINYGLTF)
    // Fallback: save as uncompressed GLB via GLTF exporter path
    const uint32_t nVerts = mesh.vertex_count();
    const uint32_t nFaces = mesh.face_count();

    std::vector<float> positions(nVerts * 3);
    float pMin[3] = { std::numeric_limits<float>::max(),  std::numeric_limits<float>::max(),  std::numeric_limits<float>::max() };
    float pMax[3] = {-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max(), -std::numeric_limits<float>::max() };

    for (uint32_t i = 0; i < nVerts; ++i)
    {
        Vertex v = mesh.get_vertex(i);
        positions[i * 3] = v.x; positions[i * 3 + 1] = v.y; positions[i * 3 + 2] = v.z;
        pMin[0] = std::min(pMin[0], v.x); pMax[0] = std::max(pMax[0], v.x);
        pMin[1] = std::min(pMin[1], v.y); pMax[1] = std::max(pMax[1], v.y);
        pMin[2] = std::min(pMin[2], v.z); pMax[2] = std::max(pMax[2], v.z);
    }

    std::vector<uint32_t> indices(nFaces * 3);
    for (uint32_t i = 0; i < nFaces; ++i)
    {
        auto f = mesh.get_face(i);
        indices[i * 3] = f[0]; indices[i * 3 + 1] = f[1]; indices[i * 3 + 2] = f[2];
    }

    tinygltf::Model model;
    tinygltf::Scene scene;
    tinygltf::Mesh gltfMesh;
    tinygltf::Primitive primitive;
    tinygltf::Buffer buffer;

    size_t posSize = positions.size() * sizeof(float);
    size_t idxSize = indices.size() * sizeof(uint32_t);
    buffer.data.resize(posSize + idxSize);
    std::memcpy(&buffer.data[0], positions.data(), posSize);
    std::memcpy(&buffer.data[posSize], indices.data(), idxSize);

    tinygltf::BufferView posView;
    posView.buffer = 0; posView.byteOffset = 0; posView.byteLength = posSize;
    posView.target = TINYGLTF_TARGET_ARRAY_BUFFER;

    tinygltf::BufferView idxView;
    idxView.buffer = 0; idxView.byteOffset = posSize; idxView.byteLength = idxSize;
    idxView.target = TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER;

    tinygltf::Accessor posAcc;
    posAcc.bufferView = 0; posAcc.byteOffset = 0;
    posAcc.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
    posAcc.count = nVerts; posAcc.type = TINYGLTF_TYPE_VEC3;
    posAcc.minValues = {pMin[0], pMin[1], pMin[2]};
    posAcc.maxValues = {pMax[0], pMax[1], pMax[2]};

    tinygltf::Accessor idxAcc;
    idxAcc.bufferView = 1; idxAcc.byteOffset = 0;
    idxAcc.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT;
    idxAcc.count = indices.size(); idxAcc.type = TINYGLTF_TYPE_SCALAR;

    primitive.attributes["POSITION"] = 0;
    primitive.indices = 1;
    primitive.mode = TINYGLTF_MODE_TRIANGLES;

    gltfMesh.primitives.push_back(primitive);
    model.meshes.push_back(gltfMesh);
    model.buffers.push_back(buffer);
    model.bufferViews.push_back(posView);
    model.bufferViews.push_back(idxView);
    model.accessors.push_back(posAcc);
    model.accessors.push_back(idxAcc);

    tinygltf::Node node; node.mesh = 0;
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

} // namespace pylmesh
