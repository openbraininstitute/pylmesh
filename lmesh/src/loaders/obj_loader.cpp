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

#include "lmesh/loaders/obj_loader.h"
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
    std::ifstream file(filepath, std::ios::binary | std::ios::ate);
    if (!file)
        return false;

    size_t size = file.tellg();
    file.seekg(0);

    std::string buffer(size, '\0');
    file.read(buffer.data(), size);

    mesh.clear();

    // Optional but VERY important for performance
    mesh.vertices.reserve(size / 30);   // heuristic
    mesh.faces.reserve(size / 60);
    // mesh.normals.reserve(size / 50);
    // mesh.texcoords.reserve(size / 50);

    const char* ptr = buffer.data();
    const char* end = ptr + buffer.size();

    auto parseFloat = [](const char*& p) -> float {
        return std::strtof(p, (char**)&p);
    };

    auto skipSpaces = [](const char*& p) {
        while (*p == ' ' || *p == '\t') ++p;
    };

    auto parseInt = [](const char*& p) -> int {
        int val = 0;
        while (*p >= '0' && *p <= '9') {
            val = val * 10 + (*p - '0');
            ++p;
        }
        return val;
    };

    while (ptr < end)
    {
        if (*ptr == '\n' || *ptr == '\r') { ++ptr; continue; }

        if (ptr[0] == 'v' && ptr[1] == ' ')
        {
            ptr += 2;
            Vertex v;
            v.x = parseFloat(ptr); skipSpaces(ptr);
            v.y = parseFloat(ptr); skipSpaces(ptr);
            v.z = parseFloat(ptr);
            mesh.vertices.push_back(v);
        }
        else if (ptr[0] == 'v' && ptr[1] == 'n')
        {
            ptr += 3;
            Normal n;
            n.nx = parseFloat(ptr); skipSpaces(ptr);
            n.ny = parseFloat(ptr); skipSpaces(ptr);
            n.nz = parseFloat(ptr);
            mesh.normals.push_back(n);
        }
        else if (ptr[0] == 'v' && ptr[1] == 't')
        {
            ptr += 3;
            TexCoord t;
            t.u = parseFloat(ptr); skipSpaces(ptr);
            t.v = parseFloat(ptr);
            mesh.texcoords.push_back(t);
        }
        else if (ptr[0] == 'f')
        {
            ptr += 2;

            Face f;

            while (ptr < end && *ptr != '\n')
            {
                skipSpaces(ptr);

                // parse index before '/'
                int idx = parseInt(ptr);

                // skip rest of vertex definition ("/uv/normals")
                while (*ptr != ' ' && *ptr != '\n' && *ptr != '\r' && ptr < end)
                    ++ptr;

                f.indices.push_back(idx - 1);
            }

            mesh.faces.push_back(f);
        }
        else
        {
            // skip unknown line
            while (ptr < end && *ptr != '\n') ++ptr;
        }

        // move to next line
        while (ptr < end && *ptr != '\n') ++ptr;
        ++ptr;
    }

    return !mesh.isEmpty();
}

} // namespace pylmesh
