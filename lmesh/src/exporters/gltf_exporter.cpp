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
#include "lmesh/ultra_quantized_mesh.h"
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

bool GLTFExporter::can_save(const std::string& filepath) const
{
    return filepath.size() >= 5 && filepath.substr(filepath.size() - 5) == ".gltf";
}

bool GLTFExporter::save(const std::string& filepath, const Mesh& mesh)
{
#ifdef PYLMESH_USE_TINYGLTF
    tinygltf::Model model;
    tinygltf::Scene scene;
    tinygltf::Mesh gltf_mesh;
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
    size_t pos_size = positions.size() * sizeof(float);
    size_t idx_size = indices.size() * sizeof(uint32_t);
    buffer.data.resize(pos_size + idx_size);
    std::memcpy(&buffer.data[0], positions.data(), pos_size);
    std::memcpy(&buffer.data[pos_size], indices.data(), idx_size);

    // Position buffer view
    tinygltf::BufferView position_view;
    position_view.buffer = 0;
    position_view.byteOffset = 0;
    position_view.byteLength = pos_size;
    position_view.target = TINYGLTF_TARGET_ARRAY_BUFFER;

    // Index buffer view
    tinygltf::BufferView index_view;
    index_view.buffer = 0;
    index_view.byteOffset = pos_size;
    index_view.byteLength = idx_size;
    index_view.target = TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER;

    // Position accessor
    tinygltf::Accessor position_accessor;
    position_accessor.bufferView = 0;
    position_accessor.byteOffset = 0;
    position_accessor.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
    position_accessor.count = mesh.vertices.size();
    position_accessor.type = TINYGLTF_TYPE_VEC3;
    position_accessor.minValues = {pMin[0], pMin[1], pMin[2]};
    position_accessor.maxValues = {pMax[0], pMax[1], pMax[2]};

    // Index accessor
    tinygltf::Accessor index_accessor;
    index_accessor.bufferView = 1;
    index_accessor.byteOffset = 0;
    index_accessor.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT;
    index_accessor.count = indices.size();
    index_accessor.type = TINYGLTF_TYPE_SCALAR;

    // Build primitive
    primitive.attributes["POSITION"] = 0;
    primitive.indices = 1;
    primitive.mode = TINYGLTF_MODE_TRIANGLES;

    // Assemble model
    gltf_mesh.primitives.push_back(primitive);
    model.meshes.push_back(gltf_mesh);
    model.buffers.push_back(buffer);
    model.bufferViews.push_back(position_view);
    model.bufferViews.push_back(index_view);
    model.accessors.push_back(position_accessor);
    model.accessors.push_back(index_accessor);

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

bool GLBExporter::can_save(const std::string& filepath) const
{
    return filepath.size() >= 4 && filepath.substr(filepath.size() - 4) == ".glb";
}

bool GLTFExporter::save(const std::string& filepath, const QuantizedMesh& mesh)
{
#ifdef PYLMESH_USE_TINYGLTF
    const uint32_t n_verts = mesh.vertex_count();
    const uint32_t n_faces = mesh.face_count();

    // Dequantize all vertices and compute bounds
    std::vector<float> positions(n_verts * 3);
    float pMin[3] = {std::numeric_limits<float>::max(), std::numeric_limits<float>::max(),
                     std::numeric_limits<float>::max()};
    float pMax[3] = {-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max(),
                     -std::numeric_limits<float>::max()};

    for (uint32_t i = 0; i < n_verts; ++i)
    {
        Vertex v = mesh.get_vertex(i);
        positions[i * 3] = v.x;
        positions[i * 3 + 1] = v.y;
        positions[i * 3 + 2] = v.z;
        pMin[0] = std::min(pMin[0], v.x);
        pMax[0] = std::max(pMax[0], v.x);
        pMin[1] = std::min(pMin[1], v.y);
        pMax[1] = std::max(pMax[1], v.y);
        pMin[2] = std::min(pMin[2], v.z);
        pMax[2] = std::max(pMax[2], v.z);
    }

    // Decode all face indices
    std::vector<uint32_t> indices(n_faces * 3);
    for (uint32_t i = 0; i < n_faces; ++i)
    {
        auto f = mesh.get_face(i);
        indices[i * 3] = f[0];
        indices[i * 3 + 1] = f[1];
        indices[i * 3 + 2] = f[2];
    }

    tinygltf::Model model;
    tinygltf::Scene scene;
    tinygltf::Mesh gltf_mesh;
    tinygltf::Primitive primitive;
    tinygltf::Buffer buffer;

    size_t pos_size = positions.size() * sizeof(float);
    size_t idx_size = indices.size() * sizeof(uint32_t);
    buffer.data.resize(pos_size + idx_size);
    std::memcpy(&buffer.data[0], positions.data(), pos_size);
    std::memcpy(&buffer.data[pos_size], indices.data(), idx_size);

    tinygltf::BufferView pos_view;
    pos_view.buffer = 0;
    pos_view.byteOffset = 0;
    pos_view.byteLength = pos_size;
    pos_view.target = TINYGLTF_TARGET_ARRAY_BUFFER;

    tinygltf::BufferView idx_view;
    idx_view.buffer = 0;
    idx_view.byteOffset = pos_size;
    idx_view.byteLength = idx_size;
    idx_view.target = TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER;

    tinygltf::Accessor pos_acc;
    pos_acc.bufferView = 0;
    pos_acc.byteOffset = 0;
    pos_acc.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
    pos_acc.count = n_verts;
    pos_acc.type = TINYGLTF_TYPE_VEC3;
    pos_acc.minValues = {pMin[0], pMin[1], pMin[2]};
    pos_acc.maxValues = {pMax[0], pMax[1], pMax[2]};

    tinygltf::Accessor idx_acc;
    idx_acc.bufferView = 1;
    idx_acc.byteOffset = 0;
    idx_acc.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT;
    idx_acc.count = indices.size();
    idx_acc.type = TINYGLTF_TYPE_SCALAR;

    primitive.attributes["POSITION"] = 0;
    primitive.indices = 1;
    primitive.mode = TINYGLTF_MODE_TRIANGLES;

    gltf_mesh.primitives.push_back(primitive);
    model.meshes.push_back(gltf_mesh);
    model.buffers.push_back(buffer);
    model.bufferViews.push_back(pos_view);
    model.bufferViews.push_back(idx_view);
    model.accessors.push_back(pos_acc);
    model.accessors.push_back(idx_acc);

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

bool GLBExporter::save(const std::string& filepath, const Mesh& mesh)
{
#if defined(PYLMESH_USE_TINYGLTF) && defined(PYLMESH_USE_DRACO)
    // Use Draco compression
    tinygltf::Model model;
    tinygltf::Scene scene;
    tinygltf::Mesh gltf_mesh;
    tinygltf::Primitive primitive;

    // Create Draco mesh
    draco::Mesh draco_mesh;
    draco_mesh.set_num_points(mesh.vertices.size());
    draco_mesh.SetNumFaces(mesh.face_count());

    // Add position attribute
    draco::GeometryAttribute pos_attr;
    pos_attr.Init(draco::GeometryAttribute::POSITION, nullptr, 3, draco::DT_FLOAT32, false,
                  sizeof(float) * 3, 0);
    int pos_att_id = draco_mesh.AddAttribute(pos_attr, true, mesh.vertices.size());

    float pMin[3] = {std::numeric_limits<float>::max(), std::numeric_limits<float>::max(),
                     std::numeric_limits<float>::max()};
    float pMax[3] = {-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max(),
                     -std::numeric_limits<float>::max()};

    // Set vertex positions
    for (size_t i = 0; i < mesh.vertices.size(); ++i)
    {
        float pos[3] = {mesh.vertices[i].x, mesh.vertices[i].y, mesh.vertices[i].z};
        draco_mesh.attribute(pos_att_id)->SetAttributeValue(draco::AttributeValueIndex(i), pos);
        pMin[0] = std::min(pMin[0], pos[0]);
        pMin[1] = std::min(pMin[1], pos[1]);
        pMin[2] = std::min(pMin[2], pos[2]);
        pMax[0] = std::max(pMax[0], pos[0]);
        pMax[1] = std::max(pMax[1], pos[1]);
        pMax[2] = std::max(pMax[2], pos[2]);
    }

    // Set faces
    for (size_t i = 0; i < mesh.face_count(); ++i)
    {
        const uint32_t* idx = mesh.face_indices(i);
        if (mesh.face_size(i) >= 3)
        {
            draco::Mesh::Face face;
            face[0] = idx[0];
            face[1] = idx[1];
            face[2] = idx[2];
            draco_mesh.SetFace(draco::FaceIndex(i), face);
        }
    }

    // Encode with Draco
    draco::Encoder encoder;
    encoder.SetSpeedOptions(5, 5);
    encoder.SetAttributeQuantization(draco::GeometryAttribute::POSITION, 14);

    draco::EncoderBuffer enc_buffer;
    if (!encoder.EncodeMeshToBuffer(draco_mesh, &enc_buffer).ok())
    {
        return false;
    }

    // Create buffer with Draco data
    tinygltf::Buffer buffer;
    buffer.data.assign(enc_buffer.data(), enc_buffer.data() + enc_buffer.size());

    tinygltf::BufferView bufferView;
    bufferView.buffer = 0;
    bufferView.byteOffset = 0;
    bufferView.byteLength = buffer.data.size();

    tinygltf::Accessor pos_accessor;
    pos_accessor.bufferView = 0;
    pos_accessor.byteOffset = 0;
    pos_accessor.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
    pos_accessor.count = mesh.vertices.size();
    pos_accessor.type = TINYGLTF_TYPE_VEC3;
    pos_accessor.minValues = {pMin[0], pMin[1], pMin[2]};
    pos_accessor.maxValues = {pMax[0], pMax[1], pMax[2]};

    primitive.attributes["POSITION"] = 0;
    primitive.mode = TINYGLTF_MODE_TRIANGLES;

    // Add Draco extension
    tinygltf::Value::Object draco_obj;
    draco_obj["bufferView"] = tinygltf::Value(0);
    tinygltf::Value::Object attrs_obj;
    attrs_obj["POSITION"] = tinygltf::Value(pos_att_id);
    draco_obj["attributes"] = tinygltf::Value(attrs_obj);
    primitive.extensions["KHR_draco_mesh_compression"] = tinygltf::Value(draco_obj);

    // Placeholder index accessor
    tinygltf::Accessor idxAccessor;
    idxAccessor.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT;
    idxAccessor.count = mesh.indices.size();
    idxAccessor.type = TINYGLTF_TYPE_SCALAR;
    idxAccessor.bufferView = -1;
    primitive.indices = 1;

    model.buffers.push_back(buffer);
    model.bufferViews.push_back(bufferView);
    model.accessors.push_back(pos_accessor);
    model.accessors.push_back(idxAccessor);

    model.extensionsUsed.push_back("KHR_draco_mesh_compression");
    model.extensionsRequired.push_back("KHR_draco_mesh_compression");

    gltf_mesh.primitives.push_back(primitive);
    model.meshes.push_back(gltf_mesh);

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
    tinygltf::Mesh gltf_mesh;
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
    size_t pos_size = positions.size() * sizeof(float);
    size_t idx_size = indices.size() * sizeof(uint32_t);
    buffer.data.resize(pos_size + idx_size);
    std::memcpy(&buffer.data[0], positions.data(), pos_size);
    std::memcpy(&buffer.data[pos_size], indices.data(), idx_size);

    // Position buffer view
    tinygltf::BufferView position_view;
    position_view.buffer = 0;
    position_view.byteOffset = 0;
    position_view.byteLength = pos_size;
    position_view.target = TINYGLTF_TARGET_ARRAY_BUFFER;

    // Index buffer view
    tinygltf::BufferView index_view;
    index_view.buffer = 0;
    index_view.byteOffset = pos_size;
    index_view.byteLength = idx_size;
    index_view.target = TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER;

    // Position accessor
    tinygltf::Accessor position_accessor;
    position_accessor.bufferView = 0;
    position_accessor.byteOffset = 0;
    position_accessor.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
    position_accessor.count = mesh.vertices.size();
    position_accessor.type = TINYGLTF_TYPE_VEC3;
    position_accessor.minValues = {pMin[0], pMin[1], pMin[2]};
    position_accessor.maxValues = {pMax[0], pMax[1], pMax[2]};

    // Index accessor
    tinygltf::Accessor index_accessor;
    index_accessor.bufferView = 1;
    index_accessor.byteOffset = 0;
    index_accessor.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT;
    index_accessor.count = indices.size();
    index_accessor.type = TINYGLTF_TYPE_SCALAR;

    // Build primitive
    primitive.attributes["POSITION"] = 0;
    primitive.indices = 1;
    primitive.mode = TINYGLTF_MODE_TRIANGLES;

    // Assemble model
    gltf_mesh.primitives.push_back(primitive);
    model.meshes.push_back(gltf_mesh);
    model.buffers.push_back(buffer);
    model.bufferViews.push_back(position_view);
    model.bufferViews.push_back(index_view);
    model.accessors.push_back(position_accessor);
    model.accessors.push_back(index_accessor);

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
    const uint32_t n_verts = mesh.vertex_count();
    const uint32_t n_faces = mesh.face_count();

    // Dequantize vertices and compute bounds
    float pMin[3] = {std::numeric_limits<float>::max(), std::numeric_limits<float>::max(),
                     std::numeric_limits<float>::max()};
    float pMax[3] = {-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max(),
                     -std::numeric_limits<float>::max()};

    draco::Mesh draco_mesh;
    draco_mesh.set_num_points(n_verts);
    draco_mesh.SetNumFaces(n_faces);

    draco::GeometryAttribute pos_attr;
    pos_attr.Init(draco::GeometryAttribute::POSITION, nullptr, 3, draco::DT_FLOAT32, false,
                  sizeof(float) * 3, 0);
    int pos_att_id = draco_mesh.AddAttribute(pos_attr, true, n_verts);

    for (uint32_t i = 0; i < n_verts; ++i)
    {
        Vertex v = mesh.get_vertex(i);
        float pos[3] = {v.x, v.y, v.z};
        draco_mesh.attribute(pos_att_id)->SetAttributeValue(draco::AttributeValueIndex(i), pos);
        pMin[0] = std::min(pMin[0], v.x);
        pMax[0] = std::max(pMax[0], v.x);
        pMin[1] = std::min(pMin[1], v.y);
        pMax[1] = std::max(pMax[1], v.y);
        pMin[2] = std::min(pMin[2], v.z);
        pMax[2] = std::max(pMax[2], v.z);
    }

    for (uint32_t i = 0; i < n_faces; ++i)
    {
        auto f = mesh.get_face(i);
        draco::Mesh::Face face;
        face[0] = f[0];
        face[1] = f[1];
        face[2] = f[2];
        draco_mesh.SetFace(draco::FaceIndex(i), face);
    }

    draco::Encoder encoder;
    encoder.SetSpeedOptions(5, 5);
    encoder.SetAttributeQuantization(draco::GeometryAttribute::POSITION, 14);

    draco::EncoderBuffer enc_buffer;
    if (!encoder.EncodeMeshToBuffer(draco_mesh, &enc_buffer).ok())
        return false;

    tinygltf::Model model;
    tinygltf::Scene scene;
    tinygltf::Mesh gltf_mesh;
    tinygltf::Primitive primitive;

    tinygltf::Buffer buffer;
    buffer.data.assign(enc_buffer.data(), enc_buffer.data() + enc_buffer.size());

    tinygltf::BufferView bufferView;
    bufferView.buffer = 0;
    bufferView.byteOffset = 0;
    bufferView.byteLength = buffer.data.size();

    tinygltf::Accessor pos_acc;
    pos_acc.bufferView = 0;
    pos_acc.byteOffset = 0;
    pos_acc.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
    pos_acc.count = n_verts;
    pos_acc.type = TINYGLTF_TYPE_VEC3;
    pos_acc.minValues = {pMin[0], pMin[1], pMin[2]};
    pos_acc.maxValues = {pMax[0], pMax[1], pMax[2]};

    primitive.attributes["POSITION"] = 0;
    primitive.mode = TINYGLTF_MODE_TRIANGLES;

    tinygltf::Value::Object draco_obj;
    draco_obj["bufferView"] = tinygltf::Value(0);
    tinygltf::Value::Object attrs_obj;
    attrs_obj["POSITION"] = tinygltf::Value(pos_att_id);
    draco_obj["attributes"] = tinygltf::Value(attrs_obj);
    primitive.extensions["KHR_draco_mesh_compression"] = tinygltf::Value(draco_obj);

    tinygltf::Accessor idx_acc;
    idx_acc.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT;
    idx_acc.count = n_faces * 3;
    idx_acc.type = TINYGLTF_TYPE_SCALAR;
    idx_acc.bufferView = -1;
    primitive.indices = 1;

    model.buffers.push_back(buffer);
    model.bufferViews.push_back(bufferView);
    model.accessors.push_back(pos_acc);
    model.accessors.push_back(idx_acc);
    model.extensionsUsed.push_back("KHR_draco_mesh_compression");
    model.extensionsRequired.push_back("KHR_draco_mesh_compression");

    gltf_mesh.primitives.push_back(primitive);
    model.meshes.push_back(gltf_mesh);

    tinygltf::Node node;
    node.mesh = 0;
    model.nodes.push_back(node);
    scene.nodes.push_back(0);
    model.scenes.push_back(scene);
    model.defaultScene = 0;

    tinygltf::TinyGLTF writer;
    return writer.WriteGltfSceneToFile(&model, filepath, false, true, true, true);
#elif defined(PYLMESH_USE_TINYGLTF)
    // Fallback: save as uncompressed GLB via GLTF exporter path
    const uint32_t n_verts = mesh.vertex_count();
    const uint32_t n_faces = mesh.face_count();

    std::vector<float> positions(n_verts * 3);
    float pMin[3] = {std::numeric_limits<float>::max(), std::numeric_limits<float>::max(),
                     std::numeric_limits<float>::max()};
    float pMax[3] = {-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max(),
                     -std::numeric_limits<float>::max()};

    for (uint32_t i = 0; i < n_verts; ++i)
    {
        Vertex v = mesh.get_vertex(i);
        positions[i * 3] = v.x;
        positions[i * 3 + 1] = v.y;
        positions[i * 3 + 2] = v.z;
        pMin[0] = std::min(pMin[0], v.x);
        pMax[0] = std::max(pMax[0], v.x);
        pMin[1] = std::min(pMin[1], v.y);
        pMax[1] = std::max(pMax[1], v.y);
        pMin[2] = std::min(pMin[2], v.z);
        pMax[2] = std::max(pMax[2], v.z);
    }

    std::vector<uint32_t> indices(n_faces * 3);
    for (uint32_t i = 0; i < n_faces; ++i)
    {
        auto f = mesh.get_face(i);
        indices[i * 3] = f[0];
        indices[i * 3 + 1] = f[1];
        indices[i * 3 + 2] = f[2];
    }

    tinygltf::Model model;
    tinygltf::Scene scene;
    tinygltf::Mesh gltf_mesh;
    tinygltf::Primitive primitive;
    tinygltf::Buffer buffer;

    size_t pos_size = positions.size() * sizeof(float);
    size_t idx_size = indices.size() * sizeof(uint32_t);
    buffer.data.resize(pos_size + idx_size);
    std::memcpy(&buffer.data[0], positions.data(), pos_size);
    std::memcpy(&buffer.data[pos_size], indices.data(), idx_size);

    tinygltf::BufferView pos_view;
    pos_view.buffer = 0;
    pos_view.byteOffset = 0;
    pos_view.byteLength = pos_size;
    pos_view.target = TINYGLTF_TARGET_ARRAY_BUFFER;

    tinygltf::BufferView idx_view;
    idx_view.buffer = 0;
    idx_view.byteOffset = pos_size;
    idx_view.byteLength = idx_size;
    idx_view.target = TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER;

    tinygltf::Accessor pos_acc;
    pos_acc.bufferView = 0;
    pos_acc.byteOffset = 0;
    pos_acc.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
    pos_acc.count = n_verts;
    pos_acc.type = TINYGLTF_TYPE_VEC3;
    pos_acc.minValues = {pMin[0], pMin[1], pMin[2]};
    pos_acc.maxValues = {pMax[0], pMax[1], pMax[2]};

    tinygltf::Accessor idx_acc;
    idx_acc.bufferView = 1;
    idx_acc.byteOffset = 0;
    idx_acc.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT;
    idx_acc.count = indices.size();
    idx_acc.type = TINYGLTF_TYPE_SCALAR;

    primitive.attributes["POSITION"] = 0;
    primitive.indices = 1;
    primitive.mode = TINYGLTF_MODE_TRIANGLES;

    gltf_mesh.primitives.push_back(primitive);
    model.meshes.push_back(gltf_mesh);
    model.buffers.push_back(buffer);
    model.bufferViews.push_back(pos_view);
    model.bufferViews.push_back(idx_view);
    model.accessors.push_back(pos_acc);
    model.accessors.push_back(idx_acc);

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

bool GLTFExporter::save(const std::string& filepath, UltraQuantizedMesh& mesh)
{
    return false; // GLTF text export not supported for UltraQuantizedMesh; use GLB
}

bool GLBExporter::save(const std::string& filepath, UltraQuantizedMesh& mesh)
{
    return false; // TODO: implement via dequantize path if needed
}

} // namespace pylmesh
