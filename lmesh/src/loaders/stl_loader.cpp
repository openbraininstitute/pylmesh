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

#include "lmesh/loaders/stl_loader.h"
#include "lmesh/quantized_mesh.h"
#include <fstream>
#include <sstream>
#include <limits>

namespace pylmesh
{

bool STLLoader::canLoad(const std::string& filepath) const
{
    return filepath.size() >= 4 && filepath.substr(filepath.size() - 4) == ".stl";
}

bool STLLoader::load(const std::string& filepath, Mesh& mesh)
{
    std::ifstream file(filepath);
    if (!file.is_open())
        return false;

    mesh.clear();
    std::string line;

    while (std::getline(file, line))
    {
        std::istringstream iss(line);
        std::string keyword;
        iss >> keyword;

        if (keyword == "vertex")
        {
            Vertex v;
            iss >> v.x >> v.y >> v.z;
            mesh.vertices.push_back(v);
        }
    }

    for (size_t i = 0; i < mesh.vertices.size(); i += 3)
    {
        uint32_t idx[3] = {(uint32_t)i, (uint32_t)i + 1, (uint32_t)i + 2};
        mesh.addFace(idx, 3);
    }

    return !mesh.isEmpty();
}

bool STLLoader::load(const std::string& filepath, QuantizedMesh& mesh)
{
    std::ifstream file(filepath);
    if (!file.is_open())
        return false;

    // Pass 1 — scan vertices to determine bounding box
    Vertex bmin{ std::numeric_limits<float>::max(),
                 std::numeric_limits<float>::max(),
                 std::numeric_limits<float>::max() };
    Vertex bmax{ std::numeric_limits<float>::lowest(),
                 std::numeric_limits<float>::lowest(),
                 std::numeric_limits<float>::lowest() };
    size_t vCount = 0;

    std::string line;
    while (std::getline(file, line))
    {
        std::istringstream iss(line);
        std::string keyword;
        iss >> keyword;
        if (keyword == "vertex")
        {
            float x, y, z;
            iss >> x >> y >> z;
            if (x < bmin.x) bmin.x = x; if (x > bmax.x) bmax.x = x;
            if (y < bmin.y) bmin.y = y; if (y > bmax.y) bmax.y = y;
            if (z < bmin.z) bmin.z = z; if (z > bmax.z) bmax.z = z;
            ++vCount;
        }
    }

    if (vCount == 0)
        return false;

    // STL has duplicate vertices — dedup=true
    QuantizedMeshBuilder builder(bmin, bmax, AxisBits::uniform(16), /*dedup=*/true);
    builder.reserve(vCount, vCount / 3);

    file.clear();
    file.seekg(0);

    std::vector<uint32_t> slots;
    slots.reserve(vCount);
    while (std::getline(file, line))
    {
        std::istringstream iss(line);
        std::string keyword;
        iss >> keyword;
        if (keyword == "vertex")
        {
            float x, y, z;
            iss >> x >> y >> z;
            slots.push_back(builder.add_vertex(x, y, z));
        }
    }

    for (size_t i = 0; i + 2 < slots.size(); i += 3)
        builder.add_face(slots[i], slots[i + 1], slots[i + 2]);

    mesh = std::move(builder).build();
    return mesh.vertex_count() > 0;
}


} // namespace pylmesh
