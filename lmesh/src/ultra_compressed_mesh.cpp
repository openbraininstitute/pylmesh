#include "lmesh/ultra_compressed_mesh.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <stdexcept>

#ifdef PYLMESH_USE_ZSTD
#include <zstd.h>
#endif

namespace pylmesh
{

// ── Varint / zigzag ─────────────────────────────────────────────────────────

static uint32_t zigzag_enc(int32_t v) noexcept { return (uint32_t(v) << 1) ^ uint32_t(v >> 31); }
static int32_t  zigzag_dec(uint32_t v) noexcept { return int32_t((v >> 1) ^ -(v & 1)); }

static void varint_write(uint32_t v, std::vector<uint8_t>& out)
{
    while (v >= 0x80) { out.push_back(uint8_t((v & 0x7F) | 0x80)); v >>= 7; }
    out.push_back(uint8_t(v));
}

static uint32_t varint_read(const uint8_t*& p) noexcept
{
    uint32_t r = 0; int s = 0;
    for (;;) { uint8_t b = *p++; r |= uint32_t(b & 0x7F) << s; if (!(b & 0x80)) return r; s += 7; }
}

// ── Morton code ─────────────────────────────────────────────────────────────

static uint64_t expand_bits(uint32_t v) noexcept
{
    uint64_t x = v & 0x1fffff;
    x = (x | (x << 32)) & 0x1f00000000ffffULL;
    x = (x | (x << 16)) & 0x1f0000ff0000ffULL;
    x = (x | (x << 8))  & 0x100f00f00f00f00fULL;
    x = (x | (x << 4))  & 0x10c30c30c30c30c3ULL;
    x = (x | (x << 2))  & 0x1249249249249249ULL;
    return x;
}

static uint64_t morton3D(uint32_t x, uint32_t y, uint32_t z) noexcept
{
    return expand_bits(x) | (expand_bits(y) << 1) | (expand_bits(z) << 2);
}

// ── Quantizer ───────────────────────────────────────────────────────────────

void Quantizer::init(const float* bmin, const float* bmax, int b)
{
    bits = b;
    const float maxval = float((1u << b) - 1u);
    for (int i = 0; i < 3; ++i)
    {
        min[i] = bmin[i];
        float range = bmax[i] - bmin[i];
        scale[i] = range > 0.f ? range / maxval : 1.f;
    }
}

void Quantizer::quantize(float x, float y, float z,
                         uint32_t& qx, uint32_t& qy, uint32_t& qz) const
{
    qx = uint32_t((x - min[0]) / scale[0] + 0.5f);
    qy = uint32_t((y - min[1]) / scale[1] + 0.5f);
    qz = uint32_t((z - min[2]) / scale[2] + 0.5f);
}

void Quantizer::dequantize(uint32_t qx, uint32_t qy, uint32_t qz,
                           float& x, float& y, float& z) const
{
    x = min[0] + float(qx) * scale[0];
    y = min[1] + float(qy) * scale[1];
    z = min[2] + float(qz) * scale[2];
}

// ── FlatLRUCache ────────────────────────────────────────────────────────────

uint32_t* FlatLRUCache::get(uint32_t chunk_id, uint32_t) noexcept
{
    for (auto& s : slots_)
    {
        if (s.chunk_id == chunk_id)
        {
            s.age = ++tick_;
            return s.data.data();
        }
    }
    return nullptr;
}

uint32_t* FlatLRUCache::put(uint32_t chunk_id, uint32_t chunk_vertex_count,
                            const uint32_t* data)
{
    // Find LRU slot (lowest age)
    Slot* victim = &slots_[0];
    for (uint32_t i = 1; i < CAPACITY; ++i)
        if (slots_[i].age < victim->age)
            victim = &slots_[i];

    victim->chunk_id = chunk_id;
    victim->age = ++tick_;
    victim->data.assign(data, data + chunk_vertex_count * 3);
    return victim->data.data();
}

// ============================================================================
//  UltraCompressedMesh
// ============================================================================

void UltraCompressedMesh::decompress_chunk(uint32_t chunk_id, uint32_t* out) const
{
    const auto& chunk = vertex_chunks_[chunk_id];

#ifdef PYLMESH_USE_ZSTD
    // Decompress zstd → varint buffer
    size_t varint_size = ZSTD_getFrameContentSize(chunk.data.data(), chunk.data.size());
    std::vector<uint8_t> varint_buf(varint_size);
    ZSTD_decompress(varint_buf.data(), varint_size, chunk.data.data(), chunk.data.size());
    const uint8_t* ptr = varint_buf.data();
#else
    const uint8_t* ptr = chunk.data.data();
#endif

    int32_t x = 0, y = 0, z = 0;
    for (uint32_t i = 0; i < chunk.count; ++i)
    {
        x += zigzag_dec(varint_read(ptr));
        y += zigzag_dec(varint_read(ptr));
        z += zigzag_dec(varint_read(ptr));
        out[i * 3]     = uint32_t(x);
        out[i * 3 + 1] = uint32_t(y);
        out[i * 3 + 2] = uint32_t(z);
    }
}

Vertex UltraCompressedMesh::get_vertex(uint32_t i)
{
    uint32_t chunk_id = i / CHUNK_SIZE;
    uint32_t local_id = i % CHUNK_SIZE;
    uint32_t count = vertex_chunks_[chunk_id].count;

    uint32_t* cached = cache_.get(chunk_id, count);
    if (!cached)
    {
        std::vector<uint32_t> tmp(count * 3);
        decompress_chunk(chunk_id, tmp.data());
        cached = cache_.put(chunk_id, count, tmp.data());
    }

    float x, y, z;
    Q_.dequantize(cached[local_id * 3], cached[local_id * 3 + 1], cached[local_id * 3 + 2], x, y, z);
    return {x, y, z};
}

UltraCompressedMesh::Face UltraCompressedMesh::get_face(uint32_t i) const
{
    if (i >= face_count())
        throw std::out_of_range("UltraCompressedMesh::get_face");
    const size_t base = size_t(i) * 3;
    return {
        static_cast<uint32_t>(indices_.get(base)),
        static_cast<uint32_t>(indices_.get(base + 1)),
        static_cast<uint32_t>(indices_.get(base + 2))
    };
}

size_t UltraCompressedMesh::vertex_bytes() const noexcept
{
    size_t total = 0;
    for (const auto& c : vertex_chunks_)
        total += c.data.size();
    return total;
}

double UltraCompressedMesh::surface_area()
{
    double area = 0.0;
    const uint32_t n = face_count();
    for (uint32_t i = 0; i < n; ++i)
    {
        const size_t base = size_t(i) * 3;
        uint32_t i0 = static_cast<uint32_t>(indices_.get(base));
        uint32_t i1 = static_cast<uint32_t>(indices_.get(base + 1));
        uint32_t i2 = static_cast<uint32_t>(indices_.get(base + 2));

        Vertex v0 = get_vertex(i0);
        Vertex v1 = get_vertex(i1);
        Vertex v2 = get_vertex(i2);

        double ax = v1.x - v0.x, ay = v1.y - v0.y, az = v1.z - v0.z;
        double bx = v2.x - v0.x, by = v2.y - v0.y, bz = v2.z - v0.z;
        double cx = ay*bz - az*by, cy = az*bx - ax*bz, cz = ax*by - ay*bx;
        area += std::sqrt(cx*cx + cy*cy + cz*cz);
    }
    return area * 0.5;
}

// ============================================================================
//  UltraCompressedMeshBuilder
// ============================================================================

uint32_t UltraCompressedMeshBuilder::index_width(uint32_t n) noexcept
{
    if (n <= 2) return 1;
    uint32_t bits = 0, v = n - 1;
    while (v) { v >>= 1; ++bits; }
    return bits;
}

UltraCompressedMeshBuilder::UltraCompressedMeshBuilder(
    Vertex min, Vertex max, int bits, bool dedup)
    : dedup_(dedup)
{
    Q_.init(&min.x, &max.x, bits);
}

void UltraCompressedMeshBuilder::reserve(size_t vertex_count, size_t face_count)
{
    tmp_qx_.reserve(vertex_count);
    tmp_qy_.reserve(vertex_count);
    tmp_qz_.reserve(vertex_count);
    tmp_indices_.reserve(face_count * 3);
    if (dedup_)
        dedup_map_.reserve(vertex_count);
}

uint32_t UltraCompressedMeshBuilder::add_vertex(float x, float y, float z)
{
    uint32_t qx, qy, qz;
    Q_.quantize(x, y, z, qx, qy, qz);

    if (dedup_)
    {
        uint64_t key = uint64_t(qx) | (uint64_t(qy) << 21) | (uint64_t(qz) << 42);
        auto [value, inserted] = dedup_map_.emplace(key, uint32_t(tmp_qx_.size()));
        if (!inserted)
            return value;
    }

    uint32_t id = uint32_t(tmp_qx_.size());
    tmp_qx_.push_back(qx);
    tmp_qy_.push_back(qy);
    tmp_qz_.push_back(qz);
    return id;
}

void UltraCompressedMeshBuilder::add_face(uint32_t a, uint32_t b, uint32_t c)
{
    tmp_indices_.push_back(a);
    tmp_indices_.push_back(b);
    tmp_indices_.push_back(c);
}

UltraCompressedMesh UltraCompressedMeshBuilder::build() &&
{
    UltraCompressedMesh mesh;
    const size_t N = tmp_qx_.size();
    mesh.vertex_count_ = uint32_t(N);
    mesh.Q_ = Q_;

    dedup_map_.clear_and_free();

    // Step 1: Morton-sort + build remap
    struct SortEntry { uint64_t morton; uint32_t orig_idx; };
    std::vector<SortEntry> sorted(N);
    for (size_t i = 0; i < N; ++i)
    {
        sorted[i].morton = morton3D(tmp_qx_[i], tmp_qy_[i], tmp_qz_[i]);
        sorted[i].orig_idx = uint32_t(i);
    }
    std::sort(sorted.begin(), sorted.end(),
              [](const SortEntry& a, const SortEntry& b) { return a.morton < b.morton; });

    std::vector<uint32_t> remap(N);
    for (size_t i = 0; i < N; ++i)
        remap[sorted[i].orig_idx] = uint32_t(i);

    // Step 2: Delta-encode + varint + zstd per chunk
    for (size_t i = 0; i < N; i += UltraCompressedMesh::CHUNK_SIZE)
    {
        size_t end = std::min(i + UltraCompressedMesh::CHUNK_SIZE, N);

        // Delta-varint encode
        std::vector<uint8_t> varint_buf;
        varint_buf.reserve((end - i) * 6);

        int32_t px = 0, py = 0, pz = 0;
        for (size_t j = i; j < end; ++j)
        {
            uint32_t oi = sorted[j].orig_idx;
            int32_t x = int32_t(tmp_qx_[oi]);
            int32_t y = int32_t(tmp_qy_[oi]);
            int32_t z = int32_t(tmp_qz_[oi]);

            varint_write(zigzag_enc(x - px), varint_buf);
            varint_write(zigzag_enc(y - py), varint_buf);
            varint_write(zigzag_enc(z - pz), varint_buf);
            px = x; py = y; pz = z;
        }

#ifdef PYLMESH_USE_ZSTD
        // Zstd compress the varint buffer
        size_t bound = ZSTD_compressBound(varint_buf.size());
        std::vector<uint8_t> compressed(bound);
        size_t csize = ZSTD_compress(compressed.data(), bound,
                                     varint_buf.data(), varint_buf.size(), 3);
        compressed.resize(csize);
        compressed.shrink_to_fit();
        mesh.vertex_chunks_.push_back({std::move(compressed), uint32_t(end - i)});
#else
        varint_buf.shrink_to_fit();
        mesh.vertex_chunks_.push_back({std::move(varint_buf), uint32_t(end - i)});
#endif
    }

    // Free temp vertex data
    { std::vector<uint32_t>{}.swap(tmp_qx_); }
    { std::vector<uint32_t>{}.swap(tmp_qy_); }
    { std::vector<uint32_t>{}.swap(tmp_qz_); }
    { std::vector<SortEntry>{}.swap(sorted); }

    // Step 3: Remap face indices
    for (auto& idx : tmp_indices_)
        idx = remap[idx];
    { std::vector<uint32_t>{}.swap(remap); }

    // Step 4: Pack face indices into BitPackedArray
    {
        const uint32_t iwidth = index_width(uint32_t(N));
        const size_t icount = tmp_indices_.size();
        mesh.indices_ = BitPackedArray(iwidth, icount);
        for (size_t i = 0; i < icount; ++i)
            mesh.indices_.set(i, tmp_indices_[i]);
        std::vector<uint32_t>{}.swap(tmp_indices_);
    }

    return mesh;
}

} // namespace pylmesh
