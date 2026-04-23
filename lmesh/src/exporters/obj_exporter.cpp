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

#include "lmesh/exporters/obj_exporter.h"
#include "lmesh/quantized_mesh.h"
#include "lmesh/ultra_quantized_mesh.h"
#include <fstream>

namespace pylmesh
{

bool OBJExporter::canSave(const std::string& filepath) const
{
    return filepath.size() >= 4 && filepath.substr(filepath.size() - 4) == ".obj";
}

bool OBJExporter::save(const std::string& filepath, const Mesh& mesh)
{
    std::ofstream file(filepath);
    if (!file.is_open())
        return false;

    file << "# OBJ file exported by pylmesh\n";

    for (const auto& v : mesh.vertices)
    {
        file << "v " << v.x << " " << v.y << " " << v.z << "\n";
    }

    for (const auto& n : mesh.normals)
    {
        file << "vn " << n.nx << " " << n.ny << " " << n.nz << "\n";
    }

    for (const auto& t : mesh.texcoords)
    {
        file << "vt " << t.u << " " << t.v << "\n";
    }

    for (size_t fi = 0; fi < mesh.face_count(); ++fi)
    {
        file << "f";
        const uint32_t* idx = mesh.face_indices(fi);
        const uint32_t n = mesh.face_size(fi);
        for (uint32_t i = 0; i < n; ++i)
        {
            file << " " << (idx[i] + 1);
        }
        file << "\n";
    }

    return true;
}

bool OBJExporter::save(const std::string& filepath, const QuantizedMesh& mesh)
{
    std::ofstream file(filepath);
    if (!file.is_open())
        return false;

    file << "# OBJ file exported by pylmesh\n";

    const uint32_t nVerts = mesh.vertex_count();
    const uint32_t nFaces = mesh.face_count();

    for (uint32_t i = 0; i < nVerts; ++i)
    {
        Vertex v = mesh.get_vertex(i);
        file << "v " << v.x << " " << v.y << " " << v.z << "\n";
    }

    for (uint32_t i = 0; i < nFaces; ++i)
    {
        auto f = mesh.get_face(i);
        file << "f " << (f[0] + 1) << " " << (f[1] + 1) << " " << (f[2] + 1) << "\n";
    }

    return true;
}

bool OBJExporter::save(const std::string& filepath, UltraQuantizedMesh& mesh)
{
    std::ofstream file(filepath);
    if (!file.is_open()) return false;
    file << "# OBJ file exported by pylmesh\n";
    for (uint32_t i = 0; i < mesh.vertex_count(); ++i) {
        Vertex v = mesh.get_vertex(i);
        file << "v " << v.x << " " << v.y << " " << v.z << "\n";
    }
    for (uint32_t i = 0; i < mesh.face_count(); ++i) {
        auto f = mesh.get_face(i);
        file << "f " << (f[0]+1) << " " << (f[1]+1) << " " << (f[2]+1) << "\n";
    }
    return true;
}

} // namespace pylmesh
