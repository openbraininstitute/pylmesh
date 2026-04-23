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

#include "lmesh/mesh.h"
#include <cmath>

#ifdef PYLMESH_USE_OPENMP
#include <omp.h>
#endif

namespace pylmesh
{

void Mesh::clear()
{
    vertices.clear();
    normals.clear();
    texcoords.clear();
    indices.clear();
    face_offsets.clear();
}

bool Mesh::is_empty() const
{
    return vertices.empty();
}


uint32_t Mesh::vertex_count() const noexcept
{
    return static_cast<uint32_t>(vertices.size());
}

uint32_t Mesh::face_count() const noexcept
{
    return static_cast<uint32_t>(face_offsets.empty() ? 0 : face_offsets.size() - 1);
}

Vertex Mesh::get_vertex(uint32_t i) const
{
    return vertices[i];
}

Mesh::Face Mesh::get_face(uint32_t i) const
{
    const uint32_t* idx = face_indices(i);
    return {idx[0], idx[1], idx[2]};
}

size_t Mesh::vertex_bytes() const noexcept
{
    return vertices.size() * sizeof(Vertex);
}

size_t Mesh::face_bytes() const noexcept
{
    return indices.size() * sizeof(uint32_t) + face_offsets.size() * sizeof(uint32_t);
}

size_t Mesh::total_bytes() const noexcept
{
    return vertex_bytes() + face_bytes();
}

uint32_t Mesh::face_size(size_t f) const
{
    assert(f < face_count());
    return face_offsets[f + 1] - face_offsets[f];
}

const uint32_t* Mesh::face_indices(size_t f) const
{
    assert(f < face_count());
    return indices.data() + face_offsets[f];
}

void Mesh::add_face(const uint32_t* idx, size_t count)
{
    if (face_offsets.empty())
        face_offsets.push_back(0);
    indices.insert(indices.end(), idx, idx + count);
    face_offsets.push_back(static_cast<uint32_t>(indices.size()));
}

void Mesh::add_face(std::initializer_list<uint32_t> idx)
{
    add_face(idx.begin(), idx.size());
}

std::vector<float> Mesh::get_vertices_array() const
{
    std::vector<float> result;
    result.reserve(vertices.size() * 3);
    for (const auto& v : vertices)
    {
        result.push_back(v.x);
        result.push_back(v.y);
        result.push_back(v.z);
    }
    return result;
}

std::vector<unsigned int> Mesh::get_faces_array() const
{
    return {indices.begin(), indices.end()};
}

double Mesh::surface_area() const
{
    const size_t n_faces = face_count();

#ifdef PYLMESH_USE_OPENMP
    double area = 0.0;
    #pragma omp parallel for reduction(+:area)
    for (size_t face_idx = 0; face_idx < n_faces; ++face_idx)
    {
        const uint32_t* idx = face_indices(face_idx);
        const uint32_t n = face_size(face_idx);
        if (n < 3)
            continue;

        const auto& v0 = vertices[idx[0]];

        for (uint32_t i = 1; i + 1 < n; ++i)
        {
            const auto& v1 = vertices[idx[i]];
            const auto& v2 = vertices[idx[i + 1]];

            double ax = static_cast<double>(v1.x) - v0.x;
            double ay = static_cast<double>(v1.y) - v0.y;
            double az = static_cast<double>(v1.z) - v0.z;

            double bx = static_cast<double>(v2.x) - v0.x;
            double by = static_cast<double>(v2.y) - v0.y;
            double bz = static_cast<double>(v2.z) - v0.z;

            double cx = ay * bz - az * by;
            double cy = az * bx - ax * bz;
            double cz = ax * by - ay * bx;

            area += 0.5 * std::sqrt(cx * cx + cy * cy + cz * cz);
        }
    }
#else
    double area = 0.0;
    for (size_t face_idx = 0; face_idx < n_faces; ++face_idx)
    {
        const uint32_t* idx = face_indices(face_idx);
        const uint32_t n = face_size(face_idx);
        if (n < 3)
            continue;

        const auto& v0 = vertices[idx[0]];

        for (uint32_t i = 1; i + 1 < n; ++i)
        {
            const auto& v1 = vertices[idx[i]];
            const auto& v2 = vertices[idx[i + 1]];

            double ax = static_cast<double>(v1.x) - v0.x;
            double ay = static_cast<double>(v1.y) - v0.y;
            double az = static_cast<double>(v1.z) - v0.z;

            double bx = static_cast<double>(v2.x) - v0.x;
            double by = static_cast<double>(v2.y) - v0.y;
            double bz = static_cast<double>(v2.z) - v0.z;

            double cx = ay * bz - az * by;
            double cy = az * bx - ax * bz;
            double cz = ax * by - ay * bx;

            area += 0.5 * std::sqrt(cx * cx + cy * cy + cz * cz);
        }
    }
#endif
    return area;
}

} // namespace pylmesh
