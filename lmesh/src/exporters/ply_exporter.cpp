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

#include "lmesh/exporters/ply_exporter.h"
#include "lmesh/quantized_mesh.h"
#include "lmesh/ultra_quantized_mesh.h"
#include <fstream>

namespace pylmesh
{

bool PLYExporter::can_save(const std::string& filepath) const
{
    return filepath.size() >= 4 && filepath.substr(filepath.size() - 4) == ".ply";
}

bool PLYExporter::save(const std::string& filepath, const Mesh& mesh)
{
    std::ofstream file(filepath);
    if (!file.is_open())
        return false;

    file << "ply\n";
    file << "format ascii 1.0\n";
    file << "element vertex " << mesh.vertices.size() << "\n";
    file << "property float x\n";
    file << "property float y\n";
    file << "property float z\n";
    file << "element face " << mesh.face_count() << "\n";
    file << "property list uchar int vertex_indices\n";
    file << "end_header\n";

    for (const auto& v : mesh.vertices)
    {
        file << v.x << " " << v.y << " " << v.z << "\n";
    }

    for (size_t fi = 0; fi < mesh.face_count(); ++fi)
    {
        const uint32_t* idx = mesh.face_indices(fi);
        const uint32_t n = mesh.face_size(fi);
        file << n;
        for (uint32_t i = 0; i < n; ++i)
        {
            file << " " << idx[i];
        }
        file << "\n";
    }

    return true;
}

bool PLYExporter::save(const std::string& filepath, const QuantizedMesh& mesh)
{
    std::ofstream file(filepath);
    if (!file.is_open())
        return false;

    const uint32_t n_verts = mesh.vertex_count();
    const uint32_t n_faces = mesh.face_count();

    file << "ply\n";
    file << "format ascii 1.0\n";
    file << "element vertex " << n_verts << "\n";
    file << "property float x\n";
    file << "property float y\n";
    file << "property float z\n";
    file << "element face " << n_faces << "\n";
    file << "property list uchar int vertex_indices\n";
    file << "end_header\n";

    for (uint32_t i = 0; i < n_verts; ++i)
    {
        Vertex v = mesh.get_vertex(i);
        file << v.x << " " << v.y << " " << v.z << "\n";
    }

    for (uint32_t i = 0; i < n_faces; ++i)
    {
        auto f = mesh.get_face(i);
        file << "3 " << f[0] << " " << f[1] << " " << f[2] << "\n";
    }

    return true;
}

bool PLYExporter::save(const std::string& filepath, UltraQuantizedMesh& mesh)
{
    std::ofstream file(filepath);
    if (!file.is_open()) return false;

    file << "ply\nformat ascii 1.0\n";
    file << "element vertex " << mesh.vertex_count() << "\n";
    file << "property float x\nproperty float y\nproperty float z\n";
    file << "element face " << mesh.face_count() << "\n";
    file << "property list uchar int vertex_indices\nend_header\n";

    for (uint32_t i = 0; i < mesh.vertex_count(); ++i) {
        Vertex v = mesh.get_vertex(i);
        file << v.x << " " << v.y << " " << v.z << "\n";
    }
    for (uint32_t i = 0; i < mesh.face_count(); ++i) {
        auto f = mesh.get_face(i);
        file << "3 " << f[0] << " " << f[1] << " " << f[2] << "\n";
    }
    return true;
}

} // namespace pylmesh
