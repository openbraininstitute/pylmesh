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

#include "lmesh/loaders/off_loader.h"
#include "lmesh/quantized_mesh.h"
#include <fstream>
#include <limits>
#include <sstream>

namespace pylmesh
{

bool OFFLoader::can_load(const std::string& filepath) const
{
    return filepath.size() >= 4 && filepath.substr(filepath.size() - 4) == ".off";
}

bool OFFLoader::load(const std::string& filepath, Mesh& mesh)
{
    std::ifstream file(filepath);
    if (!file.is_open())
        return false;

    mesh.clear();
    std::string header;
    file >> header;
    if (header != "OFF")
        return false;

    int n_vertices, n_faces, n_edges;
    file >> n_vertices >> n_faces >> n_edges;

    for (int i = 0; i < n_vertices; ++i)
    {
        Vertex v;
        file >> v.x >> v.y >> v.z;
        mesh.vertices.push_back(v);
    }

    for (int i = 0; i < n_faces; ++i)
    {
        int n;
        file >> n;
        std::vector<uint32_t> idx(n);
        for (int j = 0; j < n; ++j)
        {
            file >> idx[j];
        }
        mesh.add_face(idx.data(), idx.size());
    }

    return !mesh.is_empty();
}

bool OFFLoader::load(const std::string& filepath, QuantizedMesh& mesh)
{
    std::ifstream file(filepath);
    if (!file.is_open())
        return false;

    std::string header;
    file >> header;
    if (header != "OFF")
        return false;

    int n_vertices, n_faces, n_edges;
    file >> n_vertices >> n_faces >> n_edges;

    if (n_vertices == 0)
        return false;

    // Pass 1 — read vertices to determine bounding box
    auto data_start = file.tellg();

    Vertex bmin{std::numeric_limits<float>::max(), std::numeric_limits<float>::max(),
                std::numeric_limits<float>::max()};
    Vertex bmax{std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest(),
                std::numeric_limits<float>::lowest()};

    for (int i = 0; i < n_vertices; ++i)
    {
        float x, y, z;
        file >> x >> y >> z;
        if (x < bmin.x)
            bmin.x = x;
        if (x > bmax.x)
            bmax.x = x;
        if (y < bmin.y)
            bmin.y = y;
        if (y > bmax.y)
            bmax.y = y;
        if (z < bmin.z)
            bmin.z = z;
        if (z > bmax.z)
            bmax.z = z;
    }

    // Pass 2 — rewind to data start, add vertices and faces
    QuantizedMeshBuilder builder(bmin, bmax, AxisBits::uniform(16), /*dedup=*/false);
    builder.reserve(n_vertices, n_faces);

    file.clear();
    file.seekg(data_start);

    for (int i = 0; i < n_vertices; ++i)
    {
        float x, y, z;
        file >> x >> y >> z;
        builder.add_vertex(x, y, z);
    }

    for (int i = 0; i < n_faces; ++i)
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

bool OFFLoader::load(const std::string& filepath, UltraQuantizedMesh& mesh)
{
    std::ifstream file(filepath);
    if (!file.is_open())
        return false;
    std::string header;
    file >> header;
    if (header != "OFF")
        return false;
    int n_vertices, n_faces, n_edges;
    file >> n_vertices >> n_faces >> n_edges;
    if (n_vertices == 0)
        return false;

    auto data_start = file.tellg();
    Vertex bmin{std::numeric_limits<float>::max(), std::numeric_limits<float>::max(),
                std::numeric_limits<float>::max()};
    Vertex bmax{std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest(),
                std::numeric_limits<float>::lowest()};
    for (int i = 0; i < n_vertices; ++i)
    {
        float x, y, z;
        file >> x >> y >> z;
        if (x < bmin.x)
            bmin.x = x;
        if (x > bmax.x)
            bmax.x = x;
        if (y < bmin.y)
            bmin.y = y;
        if (y > bmax.y)
            bmax.y = y;
        if (z < bmin.z)
            bmin.z = z;
        if (z > bmax.z)
            bmax.z = z;
    }

    UltraQuantizedMeshBuilder builder(bmin, bmax, 16, /*dedup=*/false);
    builder.reserve(n_vertices, n_faces);
    file.clear();
    file.seekg(data_start);
    for (int i = 0; i < n_vertices; ++i)
    {
        float x, y, z;
        file >> x >> y >> z;
        builder.add_vertex(x, y, z);
    }
    for (int i = 0; i < n_faces; ++i)
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

} // namespace pylmesh
