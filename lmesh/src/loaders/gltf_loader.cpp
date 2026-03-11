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

bool GLTFLoader::canLoad(const std::string& filepath) const
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

        for (const auto& gltfMesh : model.meshes)
        {
            for (const auto& primitive : gltfMesh.primitives)
            {
                if (primitive.mode != TINYGLTF_MODE_TRIANGLES)
                    continue;

                size_t vertexOffset = mesh.vertices.size();

#ifdef PYLMESH_USE_DRACO
                // Check for Draco compression
                if (primitive.extensions.count("KHR_draco_mesh_compression"))
                {
                    const auto& ext = primitive.extensions.at("KHR_draco_mesh_compression");
                    int bufferViewIdx = ext.Get("bufferView").GetNumberAsInt();

                    if (bufferViewIdx >= 0 && bufferViewIdx < model.bufferViews.size())
                    {
                        const auto& bufferView = model.bufferViews[bufferViewIdx];
                        if (bufferView.buffer >= 0 && bufferView.buffer < model.buffers.size())
                        {
                            const auto& buffer = model.buffers[bufferView.buffer];
                            const uint8_t* data = &buffer.data[bufferView.byteOffset];

                            draco::DecoderBuffer decBuffer;
                            decBuffer.Init(reinterpret_cast<const char*>(data),
                                           bufferView.byteLength);

                            draco::Decoder decoder;
                            auto statusor = decoder.DecodeMeshFromBuffer(&decBuffer);

                            if (statusor.ok())
                            {
                                std::unique_ptr<draco::Mesh> dracoMesh =
                                    std::move(statusor).value();

                                // Extract positions
                                const draco::PointAttribute* posAttr = dracoMesh->GetNamedAttribute(
                                    draco::GeometryAttribute::POSITION);
                                if (posAttr)
                                {
                                    for (draco::PointIndex i(0); i < dracoMesh->num_points(); ++i)
                                    {
                                        float pos[3];
                                        posAttr->GetValue(posAttr->mapped_index(i), pos);
                                        Vertex v;
                                        v.x = pos[0];
                                        v.y = pos[1];
                                        v.z = pos[2];
                                        mesh.vertices.push_back(v);
                                    }
                                }

                                // Extract indices
                                for (draco::FaceIndex i(0); i < dracoMesh->num_faces(); ++i)
                                {
                                    const draco::Mesh::Face& face = dracoMesh->face(i);
                                    Face f;
                                    f.indices.push_back(vertexOffset + face[0].value());
                                    f.indices.push_back(vertexOffset + face[1].value());
                                    f.indices.push_back(vertexOffset + face[2].value());
                                    mesh.faces.push_back(f);
                                }

                                continue;
                            }
                        }
                    }
                }
#endif

                // Load positions
                auto posIt = primitive.attributes.find("POSITION");
                if (posIt != primitive.attributes.end())
                {
                    const auto& accessor = model.accessors[posIt->second];
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

                        Face f;
                        for (int j = 0; j < 3; ++j)
                        {
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
    }
    catch (...)
    {
        return false;
    }
#else
    return false;
#endif
}

} // namespace pylmesh
