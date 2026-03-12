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
#include <string>
#include <vector>

namespace pylmesh
{

struct Vertex
{
    float x, y, z;
};

struct Normal
{
    float nx, ny, nz;
};

struct TexCoord
{
    float u, v;
};

struct Face
{
    std::vector<unsigned int> indices;
};

class Mesh
{
  public:
    std::vector<Vertex> vertices;
    std::vector<Normal> normals;
    std::vector<TexCoord> texcoords;
    std::vector<Face> faces;

    void clear();
    bool isEmpty() const;
    size_t vertexCount() const;
    size_t faceCount() const;

    // Get vertices as flat array [x1, y1, z1, x2, y2, z2, ...]
    std::vector<float> getVerticesArray() const;

    // Get faces as flat array [i1, i2, i3, i4, i5, i6, ...]
    std::vector<unsigned int> getFacesArray() const;

    // Compute total surface area
    double surfaceArea() const;
};

} // namespace pylmesh
