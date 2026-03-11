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
#include <fstream>
#include <sstream>

namespace pylmesh
{

bool OFFLoader::canLoad(const std::string& filepath) const
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

    int nVertices, nFaces, nEdges;
    file >> nVertices >> nFaces >> nEdges;

    for (int i = 0; i < nVertices; ++i)
    {
        Vertex v;
        file >> v.x >> v.y >> v.z;
        mesh.vertices.push_back(v);
    }

    for (int i = 0; i < nFaces; ++i)
    {
        int n;
        file >> n;
        Face f;
        for (int j = 0; j < n; ++j)
        {
            unsigned int idx;
            file >> idx;
            f.indices.push_back(idx);
        }
        mesh.faces.push_back(f);
    }

    return !mesh.isEmpty();
}

} // namespace pylmesh
