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
    faceOffsets.clear();
}

bool Mesh::isEmpty() const
{
    return vertices.empty();
}

size_t Mesh::vertexCount() const
{
    return vertices.size();
}

size_t Mesh::faceCount() const
{
    return faceOffsets.empty() ? 0 : faceOffsets.size() - 1;
}

uint32_t Mesh::faceSize(size_t f) const
{
    assert(f < faceCount());
    return faceOffsets[f + 1] - faceOffsets[f];
}

const uint32_t* Mesh::faceIndices(size_t f) const
{
    assert(f < faceCount());
    return indices.data() + faceOffsets[f];
}

void Mesh::addFace(const uint32_t* idx, size_t count)
{
    if (faceOffsets.empty())
        faceOffsets.push_back(0);
    indices.insert(indices.end(), idx, idx + count);
    faceOffsets.push_back(static_cast<uint32_t>(indices.size()));
}

void Mesh::addFace(std::initializer_list<uint32_t> idx)
{
    addFace(idx.begin(), idx.size());
}

std::vector<float> Mesh::getVerticesArray() const
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

std::vector<unsigned int> Mesh::getFacesArray() const
{
    return {indices.begin(), indices.end()};
}

double Mesh::surfaceArea() const
{
    const size_t nFaces = faceCount();

#ifdef PYLMESH_USE_OPENMP
    double area = 0.0;
    #pragma omp parallel for reduction(+:area)
    for (size_t faceIdx = 0; faceIdx < nFaces; ++faceIdx)
    {
        const uint32_t* idx = faceIndices(faceIdx);
        const uint32_t n = faceSize(faceIdx);
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
    for (size_t faceIdx = 0; faceIdx < nFaces; ++faceIdx)
    {
        const uint32_t* idx = faceIndices(faceIdx);
        const uint32_t n = faceSize(faceIdx);
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
