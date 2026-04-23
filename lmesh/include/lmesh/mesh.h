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

#pragma once
#include <cassert>
#include <cstdint>
#include <initializer_list>
#include <string>
#include <vector>

#include "base_mesh.h"
#include "normal.h"
#include "tex_coord.h"
#include "vertex.h"

namespace pylmesh
{
class Mesh : public BaseMesh
{
  public:
    std::vector<Vertex> vertices;
    std::vector<Normal> normals;
    std::vector<TexCoord> texcoords;

    // Flat index buffer: [i0, i1, i2,  i0, i1, i2, i3,  ...]
    std::vector<uint32_t> indices;

    // face_offsets[f] = start of face f in indices[]
    // face_offsets[f+1] - face_offsets[f] = vertex count of face f
    // size = face_count + 1
    std::vector<uint32_t> face_offsets;

    void clear();
    bool is_empty() const;

    // BaseMesh interface
    Vertex get_vertex(uint32_t i) const override;
    Face get_face(uint32_t i) const override;
    uint32_t vertex_count() const noexcept override;
    uint32_t face_count() const noexcept override;
    double surface_area() const override;
    size_t vertex_bytes() const noexcept override;
    size_t face_bytes() const noexcept override;
    size_t total_bytes() const noexcept override;

    uint32_t face_size(size_t f) const;
    const uint32_t* face_indices(size_t f) const;

    void add_face(const uint32_t* idx, size_t count);
    void add_face(std::initializer_list<uint32_t> idx);

    std::vector<float> get_vertices_array() const;
    std::vector<unsigned int> get_faces_array() const;
};

} // namespace pylmesh
