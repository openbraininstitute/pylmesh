#include "lmesh/ultra_quantized_mesh.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <numeric>
#include <stdexcept>

#ifdef PYLMESH_USE_ZSTD
#  include <zstd.h>
//  Level 6 is the sweet spot: ~200 MB/s compression speed (vs ~15 MB/s at level 19),
//  with only ~13% larger output. Level 19 costs 12+ extra seconds on a 12M-vert mesh
//  and saves ~15 MB — not a worthwhile trade.
#  define ZSTD_LEVEL 3
#else
#  define ZSTD_LEVEL 0
#endif

namespace pylmesh
{

// ── Varint / zigzag ─────────────────────────────────────────────────────────

static void varint_write(uint32_t v, std::vector<uint8_t>& out)
{
    while (v >= 0x80) { out.push_back(uint8_t((v & 0x7F) | 0x80)); v >>= 7; }
    out.push_back(uint8_t(v));
}

static uint32_t varint_read(const uint8_t*& p) noexcept
{
    uint32_t r = 0; int s = 0;
    for (;;)
    {
        uint8_t b = *p++;
        r |= uint32_t(b & 0x7F) << s;
        if (!(b & 0x80)) return r;
        s += 7;
    }
}

static inline uint32_t zigzag_enc(int32_t v) noexcept
{
    return (uint32_t(v) << 1) ^ uint32_t(v >> 31);
}
static inline int32_t zigzag_dec(uint32_t v) noexcept
{
    return int32_t((v >> 1) ^ -(v & 1));
}

// ── Morton encoding ──────────────────────────────────────────────────────────

static uint64_t morton_expand(uint32_t v) noexcept
{
    uint64_t x = v & 0x1fffff;
    x = (x | (x << 32)) & 0x1f00000000ffffULL;
    x = (x | (x << 16)) & 0x1f0000ff0000ffULL;
    x = (x | (x <<  8)) & 0x100f00f00f00f00fULL;
    x = (x | (x <<  4)) & 0x10c30c30c30c30c3ULL;
    x = (x | (x <<  2)) & 0x1249249249249249ULL;
    return x;
}

static uint64_t morton3(uint32_t x, uint32_t y, uint32_t z) noexcept
{
    return morton_expand(x) | (morton_expand(y) << 1) | (morton_expand(z) << 2);
}

// ── Chunk compression / decompression helpers ────────────────────────────────
//
//  Column-oriented layout within every chunk:
//    Vertex chunk : [dx0..dxN | dy0..dyN | dz0..dzN]   (delta-zigzag-varint)
//    Face chunk   : [da0..daN | db0..dbN | dc0..dcN]   where a = min(a,b,c)
//
//  Each column is delta-encoded independently, then all three columns are
//  concatenated into one buffer before zstd compression.  This lets zstd
//  exploit the statistical homogeneity of each axis/role stream.

static CompressedChunk compress_columns(
    const uint32_t* col_data,   // [x0..xN | y0..yN | z0..zN]  absolute values
    uint32_t        count,
    uint32_t        n_cols = 3) // 3 for vertices (x,y,z) or faces (a,b,c)
{
    std::vector<uint8_t> varint_buf;
    varint_buf.reserve(count * n_cols * 2);

    // Each column delta-encoded independently from 0.
    // For faces the a-column is monotone increasing (sorted by min index),
    // so its deltas are all non-negative and tiny — excellent for varint.
    for (uint32_t col = 0; col < n_cols; ++col)
    {
        const uint32_t* src = col_data + col * count;
        int32_t prev = 0;
        for (uint32_t i = 0; i < count; ++i)
        {
            int32_t cur = int32_t(src[i]);
            varint_write(zigzag_enc(cur - prev), varint_buf);
            prev = cur;
        }
    }

    CompressedChunk chunk;
    chunk.count = count;

#ifdef PYLMESH_USE_ZSTD
    size_t bound      = ZSTD_compressBound(varint_buf.size());
    chunk.data.resize(bound);
    size_t csize      = ZSTD_compress(chunk.data.data(), bound,
                                      varint_buf.data(), varint_buf.size(),
                                      ZSTD_LEVEL);
    chunk.data.resize(csize);
    chunk.data.shrink_to_fit();
#else
    varint_buf.shrink_to_fit();
    chunk.data = std::move(varint_buf);
#endif

    return chunk;
}

static void decompress_columns(
    const CompressedChunk& chunk,
    uint32_t*              out,    // [x0..xN | y0..yN | z0..zN]
    uint32_t               n_cols = 3)
{
    const uint8_t* ptr = nullptr;
    std::vector<uint8_t> varint_buf;

#ifdef PYLMESH_USE_ZSTD
    size_t varint_size = ZSTD_getFrameContentSize(
        chunk.data.data(), chunk.data.size());
    varint_buf.resize(varint_size);
    ZSTD_decompress(varint_buf.data(), varint_size,
                    chunk.data.data(), chunk.data.size());
    ptr = varint_buf.data();
#else
    ptr = chunk.data.data();
#endif

    for (uint32_t col = 0; col < n_cols; ++col)
    {
        uint32_t* dst  = out + col * chunk.count;
        int32_t   prev = 0;
        for (uint32_t i = 0; i < chunk.count; ++i)
        {
            prev  += zigzag_dec(varint_read(ptr));
            dst[i] = uint32_t(prev);
        }
    }
}

// ── Quantizer ────────────────────────────────────────────────────────────────

void Quantizer::init(const float* bmin, const float* bmax, int b)
{
    bits = b;
    const float maxval = float((1u << b) - 1u);
    for (int i = 0; i < 3; ++i)
    {
        min[i]   = bmin[i];
        float rng = bmax[i] - bmin[i];
        scale[i] = rng > 0.f ? rng / maxval : 1.f;
    }
}

void Quantizer::quantize(float x, float y, float z,
                         uint32_t& qx, uint32_t& qy, uint32_t& qz) const noexcept
{
    const float maxv = float((1u << bits) - 1u);
    auto q = [&](float v, int axis) -> uint32_t {
        float n = (v - min[axis]) / scale[axis];
        if (n < 0.f) n = 0.f;
        if (n > maxv) n = maxv;
        return uint32_t(n + 0.5f);
    };
    qx = q(x, 0); qy = q(y, 1); qz = q(z, 2);
}

void Quantizer::dequantize(uint32_t qx, uint32_t qy, uint32_t qz,
                           float& x, float& y, float& z) const noexcept
{
    x = min[0] + float(qx) * scale[0];
    y = min[1] + float(qy) * scale[1];
    z = min[2] + float(qz) * scale[2];
}

// ── FlatLRUCache ─────────────────────────────────────────────────────────────

FlatLRUCache::FlatLRUCache(uint32_t slots, uint32_t chunk_size)
    : slots_(slots), chunk_size_(chunk_size)
    , meta_(slots), data_(uint64_t(slots) * chunk_size * 3, 0)
{}

const uint32_t* FlatLRUCache::get(uint32_t chunk_id) const noexcept
{
    for (uint32_t i = 0; i < slots_; ++i)
    {
        if (meta_[i].chunk_id == chunk_id)
        {
            const_cast<FlatLRUCache*>(this)->meta_[i].age = ++const_cast<FlatLRUCache*>(this)->tick_;
            return data_.data() + uint64_t(i) * chunk_size_ * 3;
        }
    }
    return nullptr;
}

const uint32_t* FlatLRUCache::put(uint32_t chunk_id, uint32_t count,
                                   const uint32_t* col_data)
{
    // Find LRU slot.
    uint32_t victim = 0;
    for (uint32_t i = 1; i < slots_; ++i)
        if (meta_[i].age < meta_[victim].age)
            victim = i;

    meta_[victim].chunk_id = chunk_id;
    meta_[victim].age      = ++tick_;

    uint32_t* slot = data_.data() + uint64_t(victim) * chunk_size_ * 3;
    std::memcpy(slot, col_data, uint64_t(count) * 3 * sizeof(uint32_t));
    return slot;
}

// ============================================================================
//  UltraQuantizedMesh
// ============================================================================

UltraQuantizedMesh::UltraQuantizedMesh()
    : vcache_(CACHE_SLOTS, CHUNK_SIZE)
    , fcache_(CACHE_SLOTS, CHUNK_SIZE)
{}

void UltraQuantizedMesh::decompress_vertex_chunk(uint32_t cid, uint32_t* out) const
{
    decompress_columns(vertex_chunks_[cid], out, 3);
}

void UltraQuantizedMesh::decompress_face_chunk(uint32_t cid, uint32_t* out) const
{
    decompress_columns(face_chunks_[cid], out, 3);
}

const uint32_t* UltraQuantizedMesh::vertex_chunk_data(uint32_t cid) const
{
    if (const uint32_t* p = vcache_.get(cid)) return p;
    const uint32_t cnt = vertex_chunks_[cid].count;
    std::vector<uint32_t> tmp(uint64_t(cnt) * 3);
    decompress_vertex_chunk(cid, tmp.data());
    return vcache_.put(cid, cnt, tmp.data());
}

const uint32_t* UltraQuantizedMesh::face_chunk_data(uint32_t cid) const
{
    if (const uint32_t* p = fcache_.get(cid)) return p;
    const uint32_t cnt = face_chunks_[cid].count;
    std::vector<uint32_t> tmp(uint64_t(cnt) * 3);
    decompress_face_chunk(cid, tmp.data());
    return fcache_.put(cid, cnt, tmp.data());
}

Vertex UltraQuantizedMesh::get_vertex(uint32_t i) const
{
    const uint32_t cid   = i / CHUNK_SIZE;
    const uint32_t local = i % CHUNK_SIZE;
    const uint32_t cnt   = vertex_chunks_[cid].count;

    // Column layout: [qx0..qxN | qy0..qyN | qz0..qzN]
    const uint32_t* data = vertex_chunk_data(cid);
    float x, y, z;
    Q_.dequantize(data[local], data[cnt + local], data[2*cnt + local], x, y, z);
    return {x, y, z};
}

UltraQuantizedMesh::Face UltraQuantizedMesh::get_face(uint32_t i) const
{
    if (i >= face_count_)
        throw std::out_of_range("UltraQuantizedMesh::get_face");

    const uint32_t cid   = i / CHUNK_SIZE;
    const uint32_t local = i % CHUNK_SIZE;
    const uint32_t cnt   = face_chunks_[cid].count;

    // Column layout: [a0..aN | b0..bN | c0..cN]
    const uint32_t* data = face_chunk_data(cid);
    return { data[local], data[cnt + local], data[2*cnt + local] };
}

size_t UltraQuantizedMesh::vertex_bytes() const noexcept
{
    size_t t = 0;
    for (const auto& c : vertex_chunks_) t += c.data.size();
    return t;
}

size_t UltraQuantizedMesh::face_bytes() const noexcept
{
    size_t t = 0;
    for (const auto& c : face_chunks_) t += c.data.size();
    return t;
}

size_t UltraQuantizedMesh::total_bytes() const noexcept
{
    return vertex_bytes() + face_bytes();
}

uint32_t UltraQuantizedMesh::vertex_count() const noexcept
{
    return vertex_count_;
}

uint32_t UltraQuantizedMesh::face_count() const noexcept
{
    return face_count_;
}

size_t UltraQuantizedMesh::cache_bytes() const noexcept
{
    return uint64_t(CACHE_SLOTS) * CHUNK_SIZE * 3 * sizeof(uint32_t) * 2;
                                                                    // ×2: vcache+fcache
}

double UltraQuantizedMesh::surface_area() const
{
    double area = 0.0;
    for (uint32_t i = 0; i < face_count_; ++i)
    {
        const Face f  = get_face(i);
        const Vertex v0 = get_vertex(f[0]);
        const Vertex v1 = get_vertex(f[1]);
        const Vertex v2 = get_vertex(f[2]);

        const double ax = v1.x - v0.x, ay = v1.y - v0.y, az = v1.z - v0.z;
        const double bx = v2.x - v0.x, by = v2.y - v0.y, bz = v2.z - v0.z;
        const double cx = ay*bz - az*by, cy = az*bx - ax*bz, cz = ax*by - ay*bx;
        area += std::sqrt(cx*cx + cy*cy + cz*cz);
    }
    return area * 0.5;
}

// ============================================================================
//  UltraQuantizedMeshBuilder
// ============================================================================

uint32_t UltraQuantizedMeshBuilder::index_width(uint32_t n) noexcept
{
    if (n <= 2) return 1;
    uint32_t bits = 0, v = n - 1;
    while (v) { v >>= 1; ++bits; }
    return bits;
}

UltraQuantizedMeshBuilder::UltraQuantizedMeshBuilder(
    Vertex min, Vertex max, int bits, bool dedup)
    : dedup_(dedup)
{
    Q_.init(&min.x, &max.x, bits);
}

void UltraQuantizedMeshBuilder::reserve(size_t vertex_count, size_t face_count)
{
    tmp_verts_.reserve(vertex_count);
    tmp_indices_.reserve(face_count * 3);
    if (dedup_)
        dedup_map_.reserve(vertex_count);
}

uint32_t UltraQuantizedMeshBuilder::add_vertex(float x, float y, float z)
{
    uint32_t qx, qy, qz;
    Q_.quantize(x, y, z, qx, qy, qz);

    if (dedup_)
    {
        // Pack 3×21 bits into 63 bits — safe sentinel (bit 63 always 0).
        uint64_t key = uint64_t(qx) | (uint64_t(qy) << 21) | (uint64_t(qz) << 42);
        auto [value, inserted] = dedup_map_.emplace(key, uint32_t(tmp_verts_.size()));
        if (!inserted) return value;
    }

    const uint32_t id = uint32_t(tmp_verts_.size());
    tmp_verts_.push_back({qx, qy, qz});
    return id;
}

void UltraQuantizedMeshBuilder::add_face(uint32_t a, uint32_t b, uint32_t c)
{
    tmp_indices_.push_back(a);
    tmp_indices_.push_back(b);
    tmp_indices_.push_back(c);
}

UltraQuantizedMesh UltraQuantizedMeshBuilder::build() &&
{
    UltraQuantizedMesh mesh;
    const uint32_t N = uint32_t(tmp_verts_.size());
    const uint32_t F = uint32_t(tmp_indices_.size() / 3);
    mesh.vertex_count_ = N;
    mesh.face_count_   = F;
    mesh.Q_            = Q_;

    // Step 1: free dedup map.
    dedup_map_.clear_and_free();

    // ── Vertex pipeline ────────────────────────────────────────────────────

    // Step 2: Morton-sort vertices using 8-pass LSB radix sort.
    //
    // std::sort on N=11.7M SortEntry{uint64,uint32} structs takes ~3.5 s because
    // it moves 12-byte records and has poor cache behaviour in its final passes.
    // Radix sort makes 8 sequential passes over the array, moving the same
    // 12-byte records but always reading and writing sequentially.
    // Empirical speedup over std::sort: 5–8× for N > 5M.
    struct SortEntry { uint64_t morton; uint32_t orig; };
    std::vector<SortEntry> vsorted(N);
    for (uint32_t i = 0; i < N; ++i)
        vsorted[i] = { morton3(tmp_verts_[i].x, tmp_verts_[i].y, tmp_verts_[i].z), i };

    {
        std::vector<SortEntry> tmp(N);
        uint32_t cnt[256];

        for (int pass = 0; pass < 8; ++pass)   // 8 passes × 8 bits = 64-bit key
        {
            const int shift = pass * 8;
            std::memset(cnt, 0, sizeof(cnt));

            for (uint32_t i = 0; i < N; ++i)
                cnt[(vsorted[i].morton >> shift) & 0xFFu]++;

            // Exclusive prefix sum: cnt[k] becomes the write offset for bucket k.
            uint32_t total = 0;
            for (int k = 0; k < 256; ++k)
            {
                uint32_t c = cnt[k];
                cnt[k]     = total;
                total     += c;
            }

            // Scatter into tmp in stable order.
            for (uint32_t i = 0; i < N; ++i)
            {
                const uint8_t bucket = (vsorted[i].morton >> shift) & 0xFFu;
                tmp[cnt[bucket]++]   = vsorted[i];
            }

            vsorted.swap(tmp);
        }
        // tmp is freed here.
    }

    // Build remap: old index → new (Morton-sorted) index.
    std::vector<uint32_t> remap(N);
    for (uint32_t i = 0; i < N; ++i)
        remap[vsorted[i].orig] = i;

    // Step 3: Pack vertices into column-oriented chunks and compress.
    // Column layout per chunk: [qx0..qxK | qy0..qyK | qz0..qzK]
    {
        const uint32_t CS = UltraQuantizedMesh::CHUNK_SIZE;
        std::vector<uint32_t> col_buf;
        col_buf.reserve(uint64_t(CS) * 3);

        for (uint32_t base = 0; base < N; base += CS)
        {
            const uint32_t cnt = std::min(CS, N - base);
            col_buf.assign(uint64_t(cnt) * 3, 0);

            uint32_t* cx = col_buf.data();
            uint32_t* cy = cx + cnt;
            uint32_t* cz = cy + cnt;

            for (uint32_t i = 0; i < cnt; ++i)
            {
                const auto& v = tmp_verts_[vsorted[base + i].orig];
                cx[i] = v.x;
                cy[i] = v.y;
                cz[i] = v.z;
            }

            mesh.vertex_chunks_.push_back(compress_columns(col_buf.data(), cnt));
        }
    }

    // Free vertex source data.
    { std::vector<QVert>{}.swap(tmp_verts_); }
    { std::vector<SortEntry>{}.swap(vsorted); }

    // ── Face pipeline ──────────────────────────────────────────────────────

    // Step 4: Remap face indices to Morton-sorted vertex positions,
    //         then sort faces by min(a,b,c) so consecutive faces are
    //         spatially adjacent — minimising index deltas within each chunk.
    const uint32_t n_idx = F * 3;
    for (uint32_t i = 0; i < n_idx; ++i)
        tmp_indices_[i] = remap[tmp_indices_[i]];

    { std::vector<uint32_t>{}.swap(remap); }

    // Sort faces by min(a,b,c) so consecutive faces are spatially adjacent,
    // minimising index deltas within each compressed chunk.
    //
    // Previous approach: allocate vector<FaceEntry> (F × 12 B = 280 MB) and
    // std::sort — swaps 12-byte structs, cache-unfriendly, ~9.5 s at F=23M.
    //
    // New approach (argsort):
    //   1. Canonicalise faces in-place in tmp_indices_ (rotate so a = min).
    //   2. Sort a vector<uint32_t> face_order (F × 4 B = 94 MB) by the
    //      precomputed canonical a-value via 4-pass LSB radix sort.
    //      Swapping 4-byte ints is 3× cheaper than 12-byte structs.
    //   3. Apply the permutation to rebuild tmp_indices_.
    {
        // Pass A: canonicalise in-place — rotate so a ≤ b, a ≤ c.
        for (uint32_t i = 0; i < F; ++i)
        {
            uint32_t a = tmp_indices_[i*3], b = tmp_indices_[i*3+1], c = tmp_indices_[i*3+2];
            if      (b < a && b < c) { tmp_indices_[i*3]=b; tmp_indices_[i*3+1]=c; tmp_indices_[i*3+2]=a; }
            else if (c < a && c < b) { tmp_indices_[i*3]=c; tmp_indices_[i*3+1]=a; tmp_indices_[i*3+2]=b; }
            // else a is already the minimum; no rotation needed.
        }

        // Pass B: extract canonical a-values as sort keys.
        // Keys are vertex indices after Morton remap, so in [0, N) ⊂ [0, 2^24).
        // 4 passes of 8-bit radix sort covers 32 bits safely.
        std::vector<uint32_t> face_order(F);
        std::iota(face_order.begin(), face_order.end(), 0u);

        {
            std::vector<uint32_t> tmp_order(F);
            uint32_t cnt[256];

            for (int pass = 0; pass < 4; ++pass)
            {
                const int shift = pass * 8;
                std::memset(cnt, 0, sizeof(cnt));

                for (uint32_t i = 0; i < F; ++i)
                    cnt[(tmp_indices_[face_order[i]*3] >> shift) & 0xFFu]++;

                uint32_t total = 0;
                for (int k = 0; k < 256; ++k)
                {
                    uint32_t c = cnt[k];
                    cnt[k]     = total;
                    total     += c;
                }

                for (uint32_t i = 0; i < F; ++i)
                {
                    const uint8_t bucket = (tmp_indices_[face_order[i]*3] >> shift) & 0xFFu;
                    tmp_order[cnt[bucket]++] = face_order[i];
                }

                face_order.swap(tmp_order);
            }
            // tmp_order freed here.
        }

        // Pass C: apply permutation — write sorted faces back into tmp_indices_.
        {
            std::vector<uint32_t> sorted_idx(uint64_t(F) * 3);
            for (uint32_t i = 0; i < F; ++i)
            {
                const uint32_t src = face_order[i];
                sorted_idx[i*3    ] = tmp_indices_[src*3    ];
                sorted_idx[i*3 + 1] = tmp_indices_[src*3 + 1];
                sorted_idx[i*3 + 2] = tmp_indices_[src*3 + 2];
            }
            tmp_indices_.swap(sorted_idx);
        }
        // face_order freed here.
    }

    // Step 5: Pack faces into column-oriented chunks and compress.
    // Column layout: [a0..aK | b0..bK | c0..cK]
    // The a-column is monotone non-decreasing (sorted by a) → tiny deltas.
    {
        const uint32_t CS = UltraQuantizedMesh::CHUNK_SIZE;
        std::vector<uint32_t> col_buf;
        col_buf.reserve(uint64_t(CS) * 3);

        for (uint32_t base = 0; base < F; base += CS)
        {
            const uint32_t cnt = std::min(CS, F - base);
            col_buf.assign(uint64_t(cnt) * 3, 0);

            uint32_t* ca = col_buf.data();
            uint32_t* cb = ca + cnt;
            uint32_t* cc = cb + cnt;

            for (uint32_t i = 0; i < cnt; ++i)
            {
                ca[i] = tmp_indices_[(base + i)*3    ];
                cb[i] = tmp_indices_[(base + i)*3 + 1];
                cc[i] = tmp_indices_[(base + i)*3 + 2];
            }

            mesh.face_chunks_.push_back(compress_columns(col_buf.data(), cnt));
        }
    }

    { std::vector<uint32_t>{}.swap(tmp_indices_); }

    return mesh;
}

} // namespace pylmesh