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
#include <limits>

#include "lmesh/loaders/obj_loader.h"
#include "lmesh/quantized_mesh.h"
#include "lmesh/mapped_file.h"
#include "fast_float.h"


namespace pylmesh
{

bool OBJLoader::can_load(const std::string& filepath) const
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

    auto skip_line = [&](const char*& p) {
        const void* nl = memchr(p, '\n', static_cast<size_t>(end - p));
        p = nl ? static_cast<const char*>(nl) + 1 : end;
    };

    auto skip_spaces = [](const char*& p) {
        while (*p == ' ' || *p == '\t') ++p;
    };

    auto parse_float = [&](const char*& p) -> float {
        float val = 0.0f;
        auto res = fast_float::from_chars(p, end, val);
        p = res.ptr;
        return val;
    };

    auto parse_index = [](const char*& p) -> int {
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

    size_t v_count     = 0;
    size_t n_count     = 0;
    size_t t_count     = 0;
    size_t face_count  = 0;
    size_t index_count = 0;

    for (const char* p = begin; p < end; )
    {
        while (p < end && (*p == ' ' || *p == '\t' || *p == '\r')) ++p;
        if (p >= end) break;

        if (p[0] == 'v')
        {
            if      (p[1] == ' ') ++v_count;
            else if (p[1] == 'n') ++n_count;
            else if (p[1] == 't') ++t_count;
        }
        else if (p[0] == 'f' && (p[1] == ' ' || p[1] == '\t'))
        {
            ++face_count;

            const char* q = p + 2;
            while (q < end && *q != '\n' && *q != '\r')
            {
                // skip spaces between tokens
                while (q < end && (*q == ' ' || *q == '\t')) ++q;
                if (*q == '\n' || *q == '\r' || q >= end) break;

                ++index_count;

                // skip the full token (v, v/vt, v/vt/vn, v//vn) — no parsing
                while (q < end && *q != ' ' && *q != '\t' && *q != '\n' && *q != '\r') ++q;
            }
        }

        skip_line(p);
    }

    // ── Exact allocation — no over-reservation, no shrink_to_fit needed ──────

    mesh.vertices.resize(v_count);
    mesh.normals.resize(n_count);       // 0 bytes if mesh has no normals
    mesh.texcoords.resize(t_count);     // 0 bytes if mesh has no texcoords
    mesh.indices.resize(index_count);
    mesh.face_offsets.resize(face_count + 1);
    mesh.face_offsets[0] = 0;

    // ─────────────────────────────────────────────────────────────────────────
    // Pass 2 — parse and fill, direct indexed writes, no push_back
    //          page cache is warm from Pass 1
    // ─────────────────────────────────────────────────────────────────────────

    size_t v_idx = 0;
    size_t n_idx = 0;
    size_t t_idx = 0;
    size_t i_idx = 0;
    size_t f_idx = 0;

    for (const char* p = begin; p < end; )
    {
        while (p < end && (*p == ' ' || *p == '\t' || *p == '\r')) ++p;
        if (p >= end) break;

        if (p[0] == 'v') [[likely]]
        {
            if (p[1] == ' ') [[likely]]
            {
                p += 2; skip_spaces(p);
                Vertex& v = mesh.vertices[v_idx++];
                v.x = parse_float(p); skip_spaces(p);
                v.y = parse_float(p); skip_spaces(p);
                v.z = parse_float(p);
            }
            else if (p[1] == 'n')
            {
                p += 3; skip_spaces(p);
                Normal& n = mesh.normals[n_idx++];
                n.nx = parse_float(p); skip_spaces(p);
                n.ny = parse_float(p); skip_spaces(p);
                n.nz = parse_float(p);
            }
            else if (p[1] == 't')
            {
                p += 3; skip_spaces(p);
                TexCoord& t = mesh.texcoords[t_idx++];
                t.u = parse_float(p); skip_spaces(p);
                t.v = parse_float(p);
            }
            else { skip_line(p); continue; }
        }
        else if (p[0] == 'f' && (p[1] == ' ' || p[1] == '\t'))
        {
            p += 2;

            while (p < end && *p != '\n' && *p != '\r')
            {
                skip_spaces(p);
                if (*p == '\n' || *p == '\r' || p >= end) break;

                int vi = parse_index(p);
                int ti = 0;

                if (*p == '/')
                {
                    ++p;
                    if (*p != '/') ti = parse_index(p);
                    if (*p == '/') { ++p; parse_index(p); /* normal unused */ }
                }

                // Resolve negative (relative) OBJ indices
                if (vi < 0) vi = static_cast<int>(v_idx) + vi + 1;
                if (ti < 0) ti = static_cast<int>(t_idx) + ti + 1;

                mesh.indices[i_idx++] = static_cast<uint32_t>(vi - 1);
                (void)ti;
            }

            mesh.face_offsets[++f_idx] = static_cast<uint32_t>(i_idx);
        }
        else [[unlikely]]
        {
            skip_line(p); continue;
        }

        skip_line(p);
    }

    return !mesh.is_empty();
}

bool OBJLoader::load(const std::string& filepath,
                     QuantizedMesh&     out_mesh)
{
        AxisBits bits = AxisBits::uniform(16);

    MappedFile file;
    if (!file.open(filepath))
        return false;
 
    const char* const begin = file.data;
    const char* const end   = begin + file.size;
 
    // -----------------------------------------------------------------------
    //  Helpers
    // -----------------------------------------------------------------------
    auto skip_line = [&](const char*& p)
    {
        const void* nl = std::memchr(p, '\n', static_cast<size_t>(end - p));
        p = nl ? static_cast<const char*>(nl) + 1 : end;
    };
 
    auto skip_spaces = [](const char*& p)
    {
        while (*p == ' ' || *p == '\t') ++p;
    };
 
    auto parse_float = [&](const char*& p) -> float
    {
        float val = 0.f;
        auto  res = fast_float::from_chars(p, end, val);
        p = res.ptr;
        return val;
    };
 
    auto parse_index = [](const char*& p) -> int
    {
        const bool neg = (*p == '-');
        if (neg) ++p;
        int val = 0;
        while (*p >= '0' && *p <= '9')
            val = val * 10 + (*p++ - '0');
        return neg ? -val : val;
    };
 
    auto skip_line_prefix = [](const char*& p, const char* end_)
    {
        while (p < end_ && (*p == ' ' || *p == '\t' || *p == '\r')) ++p;
    };
 
    // -----------------------------------------------------------------------
    //  Pass 1 — bounding box + vertex count
    // -----------------------------------------------------------------------
    Vertex bmin{ std::numeric_limits<float>::max(),
                 std::numeric_limits<float>::max(),
                 std::numeric_limits<float>::max() };
    Vertex bmax{ std::numeric_limits<float>::lowest(),
                 std::numeric_limits<float>::lowest(),
                 std::numeric_limits<float>::lowest() };
    size_t v_count = 0;
 
    for (const char* p = begin; p < end; )
    {
        skip_line_prefix(p, end);
        if (p >= end) break;
 
        if (p[0] == 'v' && p[1] == ' ')
        {
            p += 2; skip_spaces(p);
            const float x = parse_float(p); skip_spaces(p);
            const float y = parse_float(p); skip_spaces(p);
            const float z = parse_float(p);
 
            if (x < bmin.x) bmin.x = x;
            if (y < bmin.y) bmin.y = y;
            if (z < bmin.z) bmin.z = z;
            if (x > bmax.x) bmax.x = x;
            if (y > bmax.y) bmax.y = y;
            if (z > bmax.z) bmax.z = z;
            ++v_count;
        }
 
        skip_line(p);
    }
 
    if (v_count == 0)
        return false;
 
    // -----------------------------------------------------------------------
    //  Pass 2 — populate builder
    // -----------------------------------------------------------------------
    QuantizedMeshBuilder builder(bmin, bmax, bits, /*dedup=*/false);
    builder.reserve(v_count, v_count * 2);
 
    size_t v_idx = 0;
    std::vector<uint32_t> face_slots;
    face_slots.reserve(4);
 
    for (const char* p = begin; p < end; )
    {
        skip_line_prefix(p, end);
        if (p >= end) break;
 
        if (p[0] == 'v' && p[1] == ' ') [[likely]]
        {
            p += 2; skip_spaces(p);
            const float x = parse_float(p); skip_spaces(p);
            const float y = parse_float(p); skip_spaces(p);
            const float z = parse_float(p);
            builder.add_vertex(x, y, z);
            ++v_idx;
        }
        else if (p[0] == 'f' && (p[1] == ' ' || p[1] == '\t'))
        {
            p += 2;
            face_slots.clear();
 
            while (p < end && *p != '\n' && *p != '\r')
            {
                skip_spaces(p);
                if (p >= end || *p == '\n' || *p == '\r') break;
 
                int vi = parse_index(p);
 
                if (*p == '/')
                {
                    ++p;
                    if (*p != '/') parse_index(p);
                    if (*p == '/') { ++p; parse_index(p); }
                }
 
                if (vi < 0)
                    vi = static_cast<int>(v_idx) + vi + 1;
 
                face_slots.push_back(static_cast<uint32_t>(vi - 1));
            }
 
            for (size_t i = 1; i + 1 < face_slots.size(); ++i)
                builder.add_face(face_slots[0], face_slots[i], face_slots[i + 1]);
        }
 
        skip_line(p);
    }
 
    out_mesh = std::move(builder).build();
    return out_mesh.vertex_count() > 0;
}

bool OBJLoader::load(const std::string& filepath, UltraQuantizedMesh& out_mesh)
{
    MappedFile file;
    if (!file.open(filepath))
        return false;

    const char* const begin = file.data;
    const char* const end   = begin + file.size;

    auto skip_line = [&](const char*& p) {
        const void* nl = std::memchr(p, '\n', size_t(end - p));
        p = nl ? static_cast<const char*>(nl) + 1 : end;
    };
    auto skip_spaces = [](const char*& p) { while (*p == ' ' || *p == '\t') ++p; };
    auto parse_float = [&](const char*& p) -> float {
        float val = 0.f; auto res = fast_float::from_chars(p, end, val); p = res.ptr; return val;
    };
    auto parse_index = [](const char*& p) -> int {
        bool neg = (*p == '-'); if (neg) ++p;
        int val = 0; while (*p >= '0' && *p <= '9') val = val * 10 + (*p++ - '0');
        return neg ? -val : val;
    };
    auto skip_line_prefix = [](const char*& p, const char* e) {
        while (p < e && (*p == ' ' || *p == '\t' || *p == '\r')) ++p;
    };

    Vertex bmin{ std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max() };
    Vertex bmax{ std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest() };
    size_t v_count = 0;

    for (const char* p = begin; p < end; ) {
        skip_line_prefix(p, end); if (p >= end) break;
        if (p[0] == 'v' && p[1] == ' ') {
            p += 2; skip_spaces(p);
            float x = parse_float(p); skip_spaces(p);
            float y = parse_float(p); skip_spaces(p);
            float z = parse_float(p);
            if (x < bmin.x) bmin.x = x; if (x > bmax.x) bmax.x = x;
            if (y < bmin.y) bmin.y = y; if (y > bmax.y) bmax.y = y;
            if (z < bmin.z) bmin.z = z; if (z > bmax.z) bmax.z = z;
            ++v_count;
        }
        skip_line(p);
    }
    if (v_count == 0) return false;

    UltraQuantizedMeshBuilder builder(bmin, bmax, 16, /*dedup=*/false);
    builder.reserve(v_count, v_count * 2);

    size_t v_idx = 0;
    std::vector<uint32_t> face_slots;
    face_slots.reserve(4);

    for (const char* p = begin; p < end; ) {
        skip_line_prefix(p, end); if (p >= end) break;
        if (p[0] == 'v' && p[1] == ' ') [[likely]] {
            p += 2; skip_spaces(p);
            float x = parse_float(p); skip_spaces(p);
            float y = parse_float(p); skip_spaces(p);
            float z = parse_float(p);
            builder.add_vertex(x, y, z);
            ++v_idx;
        } else if (p[0] == 'f' && (p[1] == ' ' || p[1] == '\t')) {
            p += 2; face_slots.clear();
            while (p < end && *p != '\n' && *p != '\r') {
                skip_spaces(p);
                if (p >= end || *p == '\n' || *p == '\r') break;
                int vi = parse_index(p);
                if (*p == '/') { ++p; if (*p != '/') parse_index(p); if (*p == '/') { ++p; parse_index(p); } }
                if (vi < 0) vi = int(v_idx) + vi + 1;
                face_slots.push_back(uint32_t(vi - 1));
            }
            for (size_t i = 1; i + 1 < face_slots.size(); ++i)
                builder.add_face(face_slots[0], face_slots[i], face_slots[i + 1]);
        }
        skip_line(p);
    }

    out_mesh = std::move(builder).build();
    return out_mesh.vertex_count() > 0;
}


} // namespace pylmesh

// Appended at end — but we need to insert before the closing namespace brace
