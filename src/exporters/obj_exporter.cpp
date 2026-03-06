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

#include "pylmesh/exporters/obj_exporter.h"
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

    for (const auto& f : mesh.faces)
    {
        file << "f";
        for (auto idx : f.indices)
        {
            file << " " << (idx + 1);
        }
        file << "\n";
    }

    return true;
}

} // namespace pylmesh
