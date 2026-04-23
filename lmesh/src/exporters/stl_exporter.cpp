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

#include "lmesh/exporters/stl_exporter.h"
#include "lmesh/quantized_mesh.h"
#include "lmesh/ultra_quantized_mesh.h"
#include <fstream>

namespace pylmesh
{

bool STLExporter::can_save(const std::string& filepath) const
{
    return filepath.size() >= 4 && filepath.substr(filepath.size() - 4) == ".stl";
}

bool STLExporter::save(const std::string& filepath, const Mesh& mesh)
{
    std::ofstream file(filepath);
    if (!file.is_open())
        return false;

    file << "solid mesh\n";

    for (size_t fi = 0; fi < mesh.face_count(); ++fi)
    {
        const uint32_t* idx = mesh.face_indices(fi);
        const uint32_t n = mesh.face_size(fi);
        if (n >= 3)
        {
            file << "  facet normal 0 0 0\n";
            file << "    outer loop\n";
            for (size_t i = 0; i < 3; ++i)
            {
                const auto& v = mesh.vertices[idx[i]];
                file << "      vertex " << v.x << " " << v.y << " " << v.z << "\n";
            }
            file << "    endloop\n";
            file << "  endfacet\n";
        }
    }

    file << "endsolid mesh\n";
    return true;
}

bool STLExporter::save(const std::string& filepath, const QuantizedMesh& mesh)
{
    std::ofstream file(filepath);
    if (!file.is_open())
        return false;

    file << "solid mesh\n";

    const uint32_t n_faces = mesh.face_count();
    for (uint32_t i = 0; i < n_faces; ++i)
    {
        auto f = mesh.get_face(i);
        Vertex v0 = mesh.get_vertex(f[0]);
        Vertex v1 = mesh.get_vertex(f[1]);
        Vertex v2 = mesh.get_vertex(f[2]);

        file << "  facet normal 0 0 0\n";
        file << "    outer loop\n";
        file << "      vertex " << v0.x << " " << v0.y << " " << v0.z << "\n";
        file << "      vertex " << v1.x << " " << v1.y << " " << v1.z << "\n";
        file << "      vertex " << v2.x << " " << v2.y << " " << v2.z << "\n";
        file << "    endloop\n";
        file << "  endfacet\n";
    }

    file << "endsolid mesh\n";
    return true;
}

bool STLExporter::save(const std::string& filepath, UltraQuantizedMesh& mesh)
{
    std::ofstream file(filepath);
    if (!file.is_open())
        return false;
    file << "solid mesh\n";
    for (uint32_t i = 0; i < mesh.face_count(); ++i)
    {
        auto f = mesh.get_face(i);
        Vertex v0 = mesh.get_vertex(f[0]), v1 = mesh.get_vertex(f[1]), v2 = mesh.get_vertex(f[2]);
        file << "  facet normal 0 0 0\n    outer loop\n";
        file << "      vertex " << v0.x << " " << v0.y << " " << v0.z << "\n";
        file << "      vertex " << v1.x << " " << v1.y << " " << v1.z << "\n";
        file << "      vertex " << v2.x << " " << v2.y << " " << v2.z << "\n";
        file << "    endloop\n  endfacet\n";
    }
    file << "endsolid mesh\n";
    return true;
}

} // namespace pylmesh
