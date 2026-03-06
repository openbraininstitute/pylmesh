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

#include "pylmesh/loaders/obj_loader.h"
#include <fstream>
#include <sstream>

namespace pylmesh
{

bool OBJLoader::canLoad(const std::string& filepath) const
{
    return filepath.size() >= 4 && filepath.substr(filepath.size() - 4) == ".obj";
}

bool OBJLoader::load(const std::string& filepath, Mesh& mesh)
{
    std::ifstream file(filepath);
    if (!file.is_open())
        return false;

    mesh.clear();
    std::string line;

    while (std::getline(file, line))
    {
        std::istringstream iss(line);
        std::string prefix;
        iss >> prefix;

        if (prefix == "v")
        {
            Vertex v;
            iss >> v.x >> v.y >> v.z;
            mesh.vertices.push_back(v);
        }
        else if (prefix == "vn")
        {
            Normal n;
            iss >> n.nx >> n.ny >> n.nz;
            mesh.normals.push_back(n);
        }
        else if (prefix == "vt")
        {
            TexCoord t;
            iss >> t.u >> t.v;
            mesh.texcoords.push_back(t);
        }
        else if (prefix == "f")
        {
            Face f;
            std::string vertex;
            while (iss >> vertex)
            {
                unsigned int idx = std::stoi(vertex.substr(0, vertex.find('/'))) - 1;
                f.indices.push_back(idx);
            }
            mesh.faces.push_back(f);
        }
    }

    return !mesh.isEmpty();
}

} // namespace pylmesh
