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

} // namespace pylmesh
