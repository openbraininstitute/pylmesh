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

#include "lmesh/loaders/gltf_loader.h"
#include "lmesh/quantized_mesh.h"
#include <limits>

#ifdef PYLMESH_USE_TINYGLTF
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "tiny_gltf.h"
#endif

#ifdef PYLMESH_USE_DRACO
#include "draco/compression/decode.h"
#include "draco/mesh/mesh.h"
#endif

namespace pylmesh
{

bool GLTFLoader::can_load(const std::string& filepath) const
{
    size_t len = filepath.size();
    return (len >= 5 && filepath.substr(len - 5) == ".gltf") ||
           (len >= 4 && filepath.substr(len - 4) == ".glb");
}

bool GLTFLoader::load(const std::string& filepath, Mesh& mesh)
{
#ifdef PYLMESH_USE_TINYGLTF
    try
    {
        tinygltf::Model model;
        tinygltf::TinyGLTF loader;
        std::string err, warn;

        bool ret;
        if (filepath.substr(filepath.size() - 4) == ".glb")
        {
            ret = loader.LoadBinaryFromFile(&model, &err, &warn, filepath);
        }
        else
        {
            ret = loader.LoadASCIIFromFile(&model, &err, &warn, filepath);
        }

        if (!ret)
            return false;

        mesh.clear();

        for (const auto& gltf_mesh : model.meshes)
        {
            for (const auto& primitive : gltf_mesh.primitives)
            {
                if (primitive.mode != TINYGLTF_MODE_TRIANGLES)
                    continue;

                size_t vertex_offset = mesh.vertices.size();

#ifdef PYLMESH_USE_DRACO
                // Check for Draco compression
                if (primitive.extensions.count("KHR_draco_mesh_compression"))
                {
                    const auto& ext = primitive.extensions.at("KHR_draco_mesh_compression");
                    int buffer_view_idx = ext.Get("bufferView").GetNumberAsInt();

                    if (buffer_view_idx >= 0 && buffer_view_idx < model.bufferViews.size())
                    {
                        const auto& bufferView = model.bufferViews[buffer_view_idx];
                        if (bufferView.buffer >= 0 && bufferView.buffer < model.buffers.size())
                        {
                            const auto& buffer = model.buffers[bufferView.buffer];
                            const uint8_t* data = &buffer.data[bufferView.byteOffset];

                            draco::DecoderBuffer dec_buffer;
                            dec_buffer.Init(reinterpret_cast<const char*>(data),
                                            bufferView.byteLength);

                            draco::Decoder decoder;
                            auto statusor = decoder.DecodeMeshFromBuffer(&dec_buffer);

                            if (statusor.ok())
                            {
                                std::unique_ptr<draco::Mesh> draco_mesh =
                                    std::move(statusor).value();

                                // Extract positions
                                const draco::PointAttribute* pos_attr =
                                    draco_mesh->GetNamedAttribute(
                                        draco::GeometryAttribute::POSITION);
                                if (pos_attr)
                                {
                                    for (draco::PointIndex i(0); i < draco_mesh->num_points(); ++i)
                                    {
                                        float pos[3];
                                        pos_attr->GetValue(pos_attr->mapped_index(i), pos);
                                        Vertex v;
                                        v.x = pos[0];
                                        v.y = pos[1];
                                        v.z = pos[2];
                                        mesh.vertices.push_back(v);
                                    }
                                }

                                // Extract indices
                                for (draco::FaceIndex i(0); i < draco_mesh->num_faces(); ++i)
                                {
                                    const draco::Mesh::Face& face = draco_mesh->face(i);
                                    uint32_t idx[3] = {
                                        static_cast<uint32_t>(vertex_offset + face[0].value()),
                                        static_cast<uint32_t>(vertex_offset + face[1].value()),
                                        static_cast<uint32_t>(vertex_offset + face[2].value())};
                                    mesh.add_face(idx, 3);
                                }

                                continue;
                            }
                        }
                    }
                }
#endif

                // Load positions
                auto pos_it = primitive.attributes.find("POSITION");
                if (pos_it != primitive.attributes.end())
                {
                    const auto& accessor = model.accessors[pos_it->second];
                    if (accessor.bufferView < 0 || accessor.bufferView >= model.bufferViews.size())
                        continue;

                    const auto& bufferView = model.bufferViews[accessor.bufferView];
                    if (bufferView.buffer < 0 || bufferView.buffer >= model.buffers.size())
                        continue;

                    const auto& buffer = model.buffers[bufferView.buffer];
                    if (bufferView.byteOffset + accessor.byteOffset +
                            accessor.count * 3 * sizeof(float) >
                        buffer.data.size())
                        continue;

                    const float* positions = reinterpret_cast<const float*>(
                        &buffer.data[bufferView.byteOffset + accessor.byteOffset]);

                    for (size_t i = 0; i < accessor.count; ++i)
                    {
                        Vertex v;
                        v.x = positions[i * 3];
                        v.y = positions[i * 3 + 1];
                        v.z = positions[i * 3 + 2];
                        mesh.vertices.push_back(v);
                    }
                }

                // Load indices
                if (primitive.indices >= 0 && primitive.indices < model.accessors.size())
                {
                    const auto& accessor = model.accessors[primitive.indices];
                    if (accessor.bufferView < 0 || accessor.bufferView >= model.bufferViews.size())
                        continue;

                    const auto& bufferView = model.bufferViews[accessor.bufferView];
                    if (bufferView.buffer < 0 || bufferView.buffer >= model.buffers.size())
                        continue;

                    const auto& buffer = model.buffers[bufferView.buffer];
                    const uint8_t* data = &buffer.data[bufferView.byteOffset + accessor.byteOffset];

                    for (size_t i = 0; i < accessor.count; i += 3)
                    {
                        if (i + 2 >= accessor.count)
                            break;

                        uint32_t tri[3];
                        for (int j = 0; j < 3; ++j)
                        {
                            uint32_t idx;
                            if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
                                idx = reinterpret_cast<const uint16_t*>(data)[i + j];
                            else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT)
                                idx = reinterpret_cast<const uint32_t*>(data)[i + j];
                            else
                                idx = data[i + j];
                            tri[j] = static_cast<uint32_t>(vertex_offset + idx);
                        }
                        mesh.add_face(tri, 3);
                    }
                }
            }
        }

        return !mesh.is_empty();
    }
    catch (...)
    {
        return false;
    }
#else
    return false;
#endif
}

bool GLTFLoader::load(const std::string& filepath, QuantizedMesh& mesh)
{
#ifdef PYLMESH_USE_TINYGLTF
    try
    {
        tinygltf::Model model;
        tinygltf::TinyGLTF loader;
        std::string err, warn;

        bool ret;
        if (filepath.substr(filepath.size() - 4) == ".glb")
            ret = loader.LoadBinaryFromFile(&model, &err, &warn, filepath);
        else
            ret = loader.LoadASCIIFromFile(&model, &err, &warn, filepath);

        if (!ret)
            return false;

        // Helper: read a raw index from a typed buffer
        auto read_index = [](const uint8_t* data, int componentType, size_t i) -> uint32_t
        {
            if (componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
                return reinterpret_cast<const uint16_t*>(data)[i];
            else if (componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT)
                return reinterpret_cast<const uint32_t*>(data)[i];
            return data[i];
        };

#ifdef PYLMESH_USE_DRACO
        // Draco meshes must be decoded once and kept alive for both passes
        struct DecodedDraco
        {
            std::unique_ptr<draco::Mesh> mesh;
            const draco::PointAttribute* pos_attr;
        };
        std::vector<DecodedDraco> draco_meshes;
#endif

        // ─────────────────────────────────────────────────────────────────────
        // Pass 1 — scan all positions to determine bounding box
        // ─────────────────────────────────────────────────────────────────────
        Vertex bmin{std::numeric_limits<float>::max(), std::numeric_limits<float>::max(),
                    std::numeric_limits<float>::max()};
        Vertex bmax{std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest(),
                    std::numeric_limits<float>::lowest()};
        bool has_verts = false;

        auto update_bounds = [&](float x, float y, float z)
        {
            if (x < bmin.x)
                bmin.x = x;
            if (x > bmax.x)
                bmax.x = x;
            if (y < bmin.y)
                bmin.y = y;
            if (y > bmax.y)
                bmax.y = y;
            if (z < bmin.z)
                bmin.z = z;
            if (z > bmax.z)
                bmax.z = z;
            has_verts = true;
        };

        for (const auto& gltf_mesh : model.meshes)
        {
            for (const auto& primitive : gltf_mesh.primitives)
            {
                if (primitive.mode != TINYGLTF_MODE_TRIANGLES)
                    continue;

#ifdef PYLMESH_USE_DRACO
                if (primitive.extensions.count("KHR_draco_mesh_compression"))
                {
                    const auto& ext = primitive.extensions.at("KHR_draco_mesh_compression");
                    int bvIdx = ext.Get("bufferView").GetNumberAsInt();
                    const auto& bv = model.bufferViews[bvIdx];
                    const auto& buf = model.buffers[bv.buffer];

                    draco::DecoderBuffer dec_buffer;
                    dec_buffer.Init(reinterpret_cast<const char*>(&buf.data[bv.byteOffset]),
                                    bv.byteLength);

                    draco::Decoder decoder;
                    auto statusor = decoder.DecodeMeshFromBuffer(&dec_buffer);
                    if (statusor.ok())
                    {
                        auto dm = std::move(statusor).value();
                        const draco::PointAttribute* pos_attr =
                            dm->GetNamedAttribute(draco::GeometryAttribute::POSITION);
                        if (pos_attr)
                        {
                            float pos[3];
                            for (draco::PointIndex i(0); i < dm->num_points(); ++i)
                            {
                                pos_attr->GetValue(pos_attr->mapped_index(i), pos);
                                update_bounds(pos[0], pos[1], pos[2]);
                            }
                        }
                        draco_meshes.push_back({std::move(dm), pos_attr});
                        continue;
                    }
                }
#endif
                auto pos_it = primitive.attributes.find("POSITION");
                if (pos_it == primitive.attributes.end())
                    continue;

                const auto& acc = model.accessors[pos_it->second];
                const auto& bv = model.bufferViews[acc.bufferView];
                const auto& buf = model.buffers[bv.buffer];
                const float* positions =
                    reinterpret_cast<const float*>(&buf.data[bv.byteOffset + acc.byteOffset]);

                for (size_t i = 0; i < acc.count; ++i)
                    update_bounds(positions[i * 3], positions[i * 3 + 1], positions[i * 3 + 2]);
            }
        }

        if (!has_verts)
            return false;

        // ─────────────────────────────────────────────────────────────────────
        // Pass 2 — add vertices and faces one-by-one
        // ─────────────────────────────────────────────────────────────────────
        QuantizedMeshBuilder builder(bmin, bmax, AxisBits::uniform(16), /*dedup=*/false);

        uint32_t global_vertex_offset = 0;
#ifdef PYLMESH_USE_DRACO
        size_t draco_idx = 0;
#endif

        for (const auto& gltf_mesh : model.meshes)
        {
            for (const auto& primitive : gltf_mesh.primitives)
            {
                if (primitive.mode != TINYGLTF_MODE_TRIANGLES)
                    continue;

                uint32_t prim_vertex_offset = global_vertex_offset;

#ifdef PYLMESH_USE_DRACO
                if (primitive.extensions.count("KHR_draco_mesh_compression"))
                {
                    auto& dd = draco_meshes[draco_idx++];
                    const draco::PointAttribute* pos_attr =
                        dd.mesh->GetNamedAttribute(draco::GeometryAttribute::POSITION);
                    if (pos_attr)
                    {
                        float pos[3];
                        for (draco::PointIndex i(0); i < dd.mesh->num_points(); ++i)
                        {
                            pos_attr->GetValue(pos_attr->mapped_index(i), pos);
                            builder.add_vertex(pos[0], pos[1], pos[2]);
                            ++global_vertex_offset;
                        }
                    }

                    for (draco::FaceIndex i(0); i < dd.mesh->num_faces(); ++i)
                    {
                        const auto& face = dd.mesh->face(i);
                        builder.add_face(prim_vertex_offset + face[0].value(),
                                         prim_vertex_offset + face[1].value(),
                                         prim_vertex_offset + face[2].value());
                    }

                    dd.mesh.reset();
                    continue;
                }
#endif
                // Add vertices
                auto pos_it = primitive.attributes.find("POSITION");
                if (pos_it == primitive.attributes.end())
                    continue;

                const auto& pos_acc = model.accessors[pos_it->second];
                const auto& pos_bv = model.bufferViews[pos_acc.bufferView];
                const auto& pos_buf = model.buffers[pos_bv.buffer];
                const float* positions = reinterpret_cast<const float*>(
                    &pos_buf.data[pos_bv.byteOffset + pos_acc.byteOffset]);

                for (size_t i = 0; i < pos_acc.count; ++i)
                {
                    builder.add_vertex(positions[i * 3], positions[i * 3 + 1],
                                       positions[i * 3 + 2]);
                    ++global_vertex_offset;
                }

                // Add faces
                if (primitive.indices >= 0)
                {
                    const auto& idx_acc = model.accessors[primitive.indices];
                    const auto& idx_bv = model.bufferViews[idx_acc.bufferView];
                    const auto& idx_buf = model.buffers[idx_bv.buffer];
                    const uint8_t* idx_data = &idx_buf.data[idx_bv.byteOffset + idx_acc.byteOffset];

                    for (size_t i = 0; i + 2 < idx_acc.count; i += 3)
                    {
                        builder.add_face(
                            prim_vertex_offset + read_index(idx_data, idx_acc.componentType, i),
                            prim_vertex_offset + read_index(idx_data, idx_acc.componentType, i + 1),
                            prim_vertex_offset +
                                read_index(idx_data, idx_acc.componentType, i + 2));
                    }
                }
            }
        }

        mesh = std::move(builder).build();
        return mesh.vertex_count() > 0;
    }
    catch (...)
    {
        return false;
    }
#else
    return false;
#endif
}

bool GLTFLoader::load(const std::string& filepath, UltraQuantizedMesh& mesh)
{
    // Load as Mesh first, then convert via builder
    Mesh raw;
    if (!load(filepath, raw))
        return false;

    Vertex bmin{std::numeric_limits<float>::max(), std::numeric_limits<float>::max(),
                std::numeric_limits<float>::max()};
    Vertex bmax{std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest(),
                std::numeric_limits<float>::lowest()};
    for (const auto& v : raw.vertices)
    {
        if (v.x < bmin.x)
            bmin.x = v.x;
        if (v.x > bmax.x)
            bmax.x = v.x;
        if (v.y < bmin.y)
            bmin.y = v.y;
        if (v.y > bmax.y)
            bmax.y = v.y;
        if (v.z < bmin.z)
            bmin.z = v.z;
        if (v.z > bmax.z)
            bmax.z = v.z;
    }

    UltraQuantizedMeshBuilder builder(bmin, bmax, 16, /*dedup=*/false);
    builder.reserve(raw.vertices.size(), raw.face_count());
    for (const auto& v : raw.vertices)
        builder.add_vertex(v.x, v.y, v.z);
    for (size_t fi = 0; fi < raw.face_count(); ++fi)
    {
        const uint32_t* idx = raw.face_indices(fi);
        uint32_t n = raw.face_size(fi);
        for (uint32_t j = 1; j + 1 < n; ++j)
            builder.add_face(idx[0], idx[j], idx[j + 1]);
    }
    mesh = std::move(builder).build();
    return mesh.vertex_count() > 0;
}

} // namespace pylmesh
