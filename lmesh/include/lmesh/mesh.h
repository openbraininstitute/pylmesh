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

#include "normal.h"
#include "tex_coord.h"
#include "vertex.h"

namespace pylmesh
{
class Mesh
{
  public:
    std::vector<Vertex> vertices;
    std::vector<Normal> normals;
    std::vector<TexCoord> texcoords;

    // Flat index buffer: [i0, i1, i2,  i0, i1, i2, i3,  ...]
    std::vector<uint32_t> indices;

    // faceOffsets[f] = start of face f in indices[]
    // faceOffsets[f+1] - faceOffsets[f] = vertex count of face f
    // size = faceCount + 1
    std::vector<uint32_t> faceOffsets;

    void clear();
    bool isEmpty() const;
    size_t vertexCount() const;
    size_t faceCount() const;

    // Number of indices for face f
    uint32_t faceSize(size_t f) const;

    // Pointer to the first index of face f
    const uint32_t* faceIndices(size_t f) const;

    // Add a face from a pointer + count
    void addFace(const uint32_t* idx, size_t count);

    // Add a face from an initializer list
    void addFace(std::initializer_list<uint32_t> idx);

    // Get vertices as flat array [x1, y1, z1, x2, y2, z2, ...]
    std::vector<float> getVerticesArray() const;

    // Get faces as flat array [i1, i2, i3, i4, i5, i6, ...]
    std::vector<unsigned int> getFacesArray() const;

    // Compute total surface area
    double surfaceArea() const;
};

} // namespace pylmesh
