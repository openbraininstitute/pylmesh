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
#include <fstream>

namespace pylmesh
{

bool STLExporter::canSave(const std::string& filepath) const
{
    return filepath.size() >= 4 && filepath.substr(filepath.size() - 4) == ".stl";
}

bool STLExporter::save(const std::string& filepath, const Mesh& mesh)
{
    std::ofstream file(filepath);
    if (!file.is_open())
        return false;

    file << "solid mesh\n";

    for (size_t fi = 0; fi < mesh.faceCount(); ++fi)
    {
        const uint32_t* idx = mesh.faceIndices(fi);
        const uint32_t n = mesh.faceSize(fi);
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

} // namespace pylmesh
