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

#include "lmesh/loaders/ply_loader.h"
#include "lmesh/quantized_mesh.h"
#include <fstream>
#include <sstream>
#include <limits>

namespace pylmesh
{

bool PLYLoader::canLoad(const std::string& filepath) const
{
    return filepath.size() >= 4 && filepath.substr(filepath.size() - 4) == ".ply";
}

bool PLYLoader::load(const std::string& filepath, Mesh& mesh)
{
    std::ifstream file(filepath);
    if (!file.is_open())
        return false;

    mesh.clear();
    std::string line;
    int vertexCount = 0, faceCount = 0;
    bool headerDone = false;

    while (std::getline(file, line))
    {
        std::istringstream iss(line);
        std::string keyword;
        iss >> keyword;

        if (keyword == "element")
        {
            std::string type;
            int count;
            iss >> type >> count;
            if (type == "vertex")
                vertexCount = count;
            if (type == "face")
                faceCount = count;
        }
        else if (keyword == "end_header")
        {
            headerDone = true;
            break;
        }
    }

    if (headerDone)
    {
        for (int i = 0; i < vertexCount; ++i)
        {
            Vertex v;
            file >> v.x >> v.y >> v.z;
            mesh.vertices.push_back(v);
            std::getline(file, line);
        }

        for (int i = 0; i < faceCount; ++i)
        {
            int n;
            file >> n;
            std::vector<uint32_t> idx(n);
            for (int j = 0; j < n; ++j)
            {
                file >> idx[j];
            }
            mesh.addFace(idx.data(), idx.size());
        }
    }

    return !mesh.isEmpty();
}

bool PLYLoader::load(const std::string& filepath, QuantizedMesh& mesh)
{
    std::ifstream file(filepath);
    if (!file.is_open())
        return false;

    std::string line;
    int vertexCount = 0, faceCount = 0;

    // Parse header
    while (std::getline(file, line))
    {
        std::istringstream iss(line);
        std::string keyword;
        iss >> keyword;
        if (keyword == "element")
        {
            std::string type; int count;
            iss >> type >> count;
            if (type == "vertex") vertexCount = count;
            if (type == "face")   faceCount = count;
        }
        else if (keyword == "end_header")
            break;
    }

    if (vertexCount == 0)
        return false;

    // Pass 1 — read vertices to determine bounding box
    auto dataStart = file.tellg();

    Vertex bmin{ std::numeric_limits<float>::max(),
                 std::numeric_limits<float>::max(),
                 std::numeric_limits<float>::max() };
    Vertex bmax{ std::numeric_limits<float>::lowest(),
                 std::numeric_limits<float>::lowest(),
                 std::numeric_limits<float>::lowest() };

    for (int i = 0; i < vertexCount; ++i)
    {
        float x, y, z;
        file >> x >> y >> z;
        if (x < bmin.x) bmin.x = x; if (x > bmax.x) bmax.x = x;
        if (y < bmin.y) bmin.y = y; if (y > bmax.y) bmax.y = y;
        if (z < bmin.z) bmin.z = z; if (z > bmax.z) bmax.z = z;
        std::getline(file, line);
    }

    // Pass 2 — rewind to data start, add vertices and faces
    QuantizedMeshBuilder builder(bmin, bmax, AxisBits::uniform(16), /*dedup=*/false);
    builder.reserve(vertexCount, faceCount);

    file.clear();
    file.seekg(dataStart);

    for (int i = 0; i < vertexCount; ++i)
    {
        float x, y, z;
        file >> x >> y >> z;
        builder.add_vertex(x, y, z);
        std::getline(file, line);
    }

    for (int i = 0; i < faceCount; ++i)
    {
        int n;
        file >> n;
        std::vector<uint32_t> idx(n);
        for (int j = 0; j < n; ++j)
            file >> idx[j];

        for (int j = 1; j + 1 < n; ++j)
            builder.add_face(idx[0], idx[j], idx[j + 1]);
    }

    mesh = std::move(builder).build();
    return mesh.vertex_count() > 0;
}

bool PLYLoader::load(const std::string& filepath, UltraCompressedMesh& mesh)
{
    std::ifstream file(filepath);
    if (!file.is_open()) return false;

    std::string line;
    int vertexCount = 0, faceCount = 0;
    while (std::getline(file, line)) {
        std::istringstream iss(line); std::string kw; iss >> kw;
        if (kw == "element") { std::string t; int c; iss >> t >> c; if (t == "vertex") vertexCount = c; if (t == "face") faceCount = c; }
        else if (kw == "end_header") break;
    }
    if (vertexCount == 0) return false;

    auto dataStart = file.tellg();
    Vertex bmin{std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max()};
    Vertex bmax{std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest()};
    for (int i = 0; i < vertexCount; ++i) {
        float x, y, z; file >> x >> y >> z;
        if (x < bmin.x) bmin.x = x; if (x > bmax.x) bmax.x = x;
        if (y < bmin.y) bmin.y = y; if (y > bmax.y) bmax.y = y;
        if (z < bmin.z) bmin.z = z; if (z > bmax.z) bmax.z = z;
        std::getline(file, line);
    }

    UltraCompressedMeshBuilder builder(bmin, bmax, 16, /*dedup=*/false);
    builder.reserve(vertexCount, faceCount);
    file.clear(); file.seekg(dataStart);
    for (int i = 0; i < vertexCount; ++i) {
        float x, y, z; file >> x >> y >> z; builder.add_vertex(x, y, z); std::getline(file, line);
    }
    for (int i = 0; i < faceCount; ++i) {
        int n; file >> n; std::vector<uint32_t> idx(n);
        for (int j = 0; j < n; ++j) file >> idx[j];
        for (int j = 1; j + 1 < n; ++j) builder.add_face(idx[0], idx[j], idx[j + 1]);
    }
    mesh = std::move(builder).build();
    return mesh.vertex_count() > 0;
}

} // namespace pylmesh
