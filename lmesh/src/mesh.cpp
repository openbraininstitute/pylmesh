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
    faces.clear();
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
    return faces.size();
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
    std::vector<unsigned int> result;
    for (const auto& f : faces)
    {
        for (auto idx : f.indices)
        {
            result.push_back(idx);
        }
    }
    return result;
}

double Mesh::surfaceArea() const
{
#ifdef PYLMESH_USE_OPENMP
    const int numThreads = omp_get_max_threads();
    omp_set_num_threads(numThreads);
    
    double area = 0.0;
    #pragma omp parallel for reduction(+:area)
    for (size_t faceIdx = 0; faceIdx < faces.size(); ++faceIdx)
    {
        const auto& face = faces[faceIdx];
        if (face.indices.size() < 3)
            continue;

        const auto& v0 = vertices[face.indices[0]];

        for (size_t i = 1; i + 1 < face.indices.size(); ++i)
        {
            const auto& v1 = vertices[face.indices[i]];
            const auto& v2 = vertices[face.indices[i + 1]];

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
    // Serial fallback when OpenMP is not available
    double area = 0.0;
    for (const auto& face : faces)
    {
        if (face.indices.size() < 3)
            continue;

        const auto& v0 = vertices[face.indices[0]];

        for (size_t i = 1; i + 1 < face.indices.size(); ++i)
        {
            const auto& v1 = vertices[face.indices[i]];
            const auto& v2 = vertices[face.indices[i + 1]];

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
