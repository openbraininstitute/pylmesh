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

 
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <charconv>

#include "lmesh/loaders/obj_loader.h"
#include "lmesh/mapped_file.h"
#include "fast_float.h"


namespace pylmesh
{

bool OBJLoader::canLoad(const std::string& filepath) const
{
    return filepath.size() >= 4 && filepath.substr(filepath.size() - 4) == ".obj";
}

bool OBJLoader::load(const std::string& filepath, Mesh& mesh)
{
    MappedFile file;
    if (!file.open(filepath))
        return false;

    const char* const begin = file.data;
    const char* const end   = begin + file.size;

    mesh.clear();

    // ─────────────────────────────────────────────────────────────────────────
    // Helpers
    // ─────────────────────────────────────────────────────────────────────────

    auto skipLine = [&](const char*& p) {
        const void* nl = memchr(p, '\n', static_cast<size_t>(end - p));
        p = nl ? static_cast<const char*>(nl) + 1 : end;
    };

    auto skipSpaces = [](const char*& p) {
        while (*p == ' ' || *p == '\t') ++p;
    };

    auto parseFloat = [&](const char*& p) -> float {
        float val = 0.0f;
        auto res = fast_float::from_chars(p, end, val);
        p = res.ptr;
        return val;
    };

    auto parseIndex = [](const char*& p) -> int {
        bool neg = (*p == '-');
        if (neg) ++p;
        int val = 0;
        while (*p >= '0' && *p <= '9')
            val = val * 10 + (*p++ - '0');
        return neg ? -val : val;
    };

    // ─────────────────────────────────────────────────────────────────────────
    // Pass 1 — count only, no float parsing, no allocation
    //          memchr jumps to '\n' in SIMD chunks — very cheap per line
    // ─────────────────────────────────────────────────────────────────────────

    size_t vCount     = 0;
    size_t nCount     = 0;
    size_t tCount     = 0;
    size_t faceCount  = 0;
    size_t indexCount = 0;

    for (const char* p = begin; p < end; )
    {
        while (p < end && (*p == ' ' || *p == '\t' || *p == '\r')) ++p;
        if (p >= end) break;

        if (p[0] == 'v')
        {
            if      (p[1] == ' ') ++vCount;
            else if (p[1] == 'n') ++nCount;
            else if (p[1] == 't') ++tCount;
        }
        else if (p[0] == 'f' && (p[1] == ' ' || p[1] == '\t'))
        {
            ++faceCount;

            const char* q = p + 2;
            while (q < end && *q != '\n' && *q != '\r')
            {
                // skip spaces between tokens
                while (q < end && (*q == ' ' || *q == '\t')) ++q;
                if (*q == '\n' || *q == '\r' || q >= end) break;

                ++indexCount;

                // skip the full token (v, v/vt, v/vt/vn, v//vn) — no parsing
                while (q < end && *q != ' ' && *q != '\t' && *q != '\n' && *q != '\r') ++q;
            }
        }

        skipLine(p);
    }

    // ── Exact allocation — no over-reservation, no shrink_to_fit needed ──────

    mesh.vertices.resize(vCount);
    mesh.normals.resize(nCount);       // 0 bytes if mesh has no normals
    mesh.texcoords.resize(tCount);     // 0 bytes if mesh has no texcoords
    mesh.indices.resize(indexCount);
    mesh.faceOffsets.resize(faceCount + 1);
    mesh.faceOffsets[0] = 0;

    // ─────────────────────────────────────────────────────────────────────────
    // Pass 2 — parse and fill, direct indexed writes, no push_back
    //          page cache is warm from Pass 1
    // ─────────────────────────────────────────────────────────────────────────

    size_t vIdx = 0;
    size_t nIdx = 0;
    size_t tIdx = 0;
    size_t iIdx = 0;
    size_t fIdx = 0;

    for (const char* p = begin; p < end; )
    {
        while (p < end && (*p == ' ' || *p == '\t' || *p == '\r')) ++p;
        if (p >= end) break;

        if (p[0] == 'v') [[likely]]
        {
            if (p[1] == ' ') [[likely]]
            {
                p += 2;
                Vertex& v = mesh.vertices[vIdx++];
                v.x = parseFloat(p); skipSpaces(p);
                v.y = parseFloat(p); skipSpaces(p);
                v.z = parseFloat(p);
            }
            else if (p[1] == 'n')
            {
                p += 3;
                Normal& n = mesh.normals[nIdx++];
                n.nx = parseFloat(p); skipSpaces(p);
                n.ny = parseFloat(p); skipSpaces(p);
                n.nz = parseFloat(p);
            }
            else if (p[1] == 't')
            {
                p += 3;
                TexCoord& t = mesh.texcoords[tIdx++];
                t.u = parseFloat(p); skipSpaces(p);
                t.v = parseFloat(p);
            }
            else { skipLine(p); continue; }
        }
        else if (p[0] == 'f' && (p[1] == ' ' || p[1] == '\t'))
        {
            p += 2;

            while (p < end && *p != '\n' && *p != '\r')
            {
                skipSpaces(p);
                if (*p == '\n' || *p == '\r' || p >= end) break;

                int vi = parseIndex(p);
                int ti = 0;

                if (*p == '/')
                {
                    ++p;
                    if (*p != '/') ti = parseIndex(p);
                    if (*p == '/') { ++p; parseIndex(p); /* normal unused */ }
                }

                // Resolve negative (relative) OBJ indices
                if (vi < 0) vi = static_cast<int>(vIdx) + vi + 1;
                if (ti < 0) ti = static_cast<int>(tIdx) + ti + 1;

                mesh.indices[iIdx++] = static_cast<uint32_t>(vi - 1);
                (void)ti;
            }

            mesh.faceOffsets[++fIdx] = static_cast<uint32_t>(iIdx);
        }
        else [[unlikely]]
        {
            skipLine(p); continue;
        }

        skipLine(p);
    }

    return !mesh.isEmpty();
}

} // namespace pylmesh
