#include "lmesh/ultra_compressed_mesh.h"

#include <algorithm>
#include <cmath>
#include <numeric>
#include <stdexcept>

namespace pylmesh
{

// ── Varint / zigzag helpers ─────────────────────────────────────────────────

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

void Quantizer::init(const float* bmin, const float* bmax, int bits)
{
    const float maxval = float((1u << bits) - 1u);
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

// ── LRUCache ────────────────────────────────────────────────────────────────

LRUCache::LRUCache(size_t capacity) : capacity_(capacity) {}

bool LRUCache::get(uint32_t key, DecompressedChunk*& value)
{
    auto it = map_.find(key);
    if (it == map_.end()) return false;
    order_.splice(order_.begin(), order_, it->second.first);
    value = &it->second.second;
    return true;
}

void LRUCache::put(uint32_t key, DecompressedChunk&& value)
{
    if (map_.size() >= capacity_)
    {
        uint32_t old = order_.back();
        order_.pop_back();
        map_.erase(old);
    }
    order_.push_front(key);
    map_[key] = {order_.begin(), std::move(value)};
}

// ============================================================================
//  UltraCompressedMesh
// ============================================================================

DecompressedChunk UltraCompressedMesh::decompress_chunk(uint32_t chunk_id)
{
    const auto& chunk = vertex_chunks_[chunk_id];
    DecompressedChunk out;
    out.vertices.reserve(chunk.count * 3);

    const uint8_t* ptr = chunk.data.data();
    int32_t x = 0, y = 0, z = 0;

    for (uint32_t i = 0; i < chunk.count; ++i)
    {
        x += zigzag_dec(varint_read(ptr));
        y += zigzag_dec(varint_read(ptr));
        z += zigzag_dec(varint_read(ptr));
        out.vertices.push_back(uint32_t(x));
        out.vertices.push_back(uint32_t(y));
        out.vertices.push_back(uint32_t(z));
    }
    return out;
}

Vertex UltraCompressedMesh::get_vertex(uint32_t i)
{
    // Remap from original index to Morton-sorted index
    uint32_t sorted_i = remap_.empty() ? i : remap_[i];

    uint32_t chunk_id = sorted_i / CHUNK_SIZE;
    uint32_t local_id = sorted_i % CHUNK_SIZE;

    DecompressedChunk* chunk;
    if (!cache_.get(chunk_id, chunk))
    {
        cache_.put(chunk_id, decompress_chunk(chunk_id));
        cache_.get(chunk_id, chunk);
    }

    const uint32_t* v = &chunk->vertices[local_id * 3];
    float x, y, z;
    Q_.dequantize(v[0], v[1], v[2], x, y, z);
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
        auto f = get_face(i);
        Vertex v0 = get_vertex(f[0]);
        Vertex v1 = get_vertex(f[1]);
        Vertex v2 = get_vertex(f[2]);

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
    : bmin_(min), bmax_(max), bits_(bits), dedup_(dedup)
{}

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
    Quantizer Q;
    Q.init(&bmin_.x, &bmax_.x, bits_);

    uint32_t qx, qy, qz;
    Q.quantize(x, y, z, qx, qy, qz);

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

    // Init quantizer on the sealed mesh
    mesh.Q_.init(&bmin_.x, &bmax_.x, bits_);

    // Free dedup map
    dedup_map_.clear_and_free();

    // Step 1: Morton-sort vertices and build remap table
    struct SortEntry { uint64_t morton; uint32_t orig_idx; };
    std::vector<SortEntry> sorted(N);
    for (size_t i = 0; i < N; ++i)
    {
        sorted[i].morton = morton3D(tmp_qx_[i], tmp_qy_[i], tmp_qz_[i]);
        sorted[i].orig_idx = uint32_t(i);
    }
    std::sort(sorted.begin(), sorted.end(),
              [](const SortEntry& a, const SortEntry& b) { return a.morton < b.morton; });

    // remap_[original_index] = sorted_index
    mesh.remap_.resize(N);
    for (size_t i = 0; i < N; ++i)
        mesh.remap_[sorted[i].orig_idx] = uint32_t(i);

    // Step 2: Delta-encode + varint-compress in chunks
    for (size_t i = 0; i < N; i += UltraCompressedMesh::CHUNK_SIZE)
    {
        size_t end = std::min(i + UltraCompressedMesh::CHUNK_SIZE, N);
        std::vector<uint8_t> encoded;
        encoded.reserve((end - i) * 6);

        int32_t px = 0, py = 0, pz = 0;
        for (size_t j = i; j < end; ++j)
        {
            uint32_t oi = sorted[j].orig_idx;
            int32_t x = int32_t(tmp_qx_[oi]);
            int32_t y = int32_t(tmp_qy_[oi]);
            int32_t z = int32_t(tmp_qz_[oi]);

            varint_write(zigzag_enc(x - px), encoded);
            varint_write(zigzag_enc(y - py), encoded);
            varint_write(zigzag_enc(z - pz), encoded);

            px = x; py = y; pz = z;
        }
        mesh.vertex_chunks_.push_back({std::move(encoded), uint32_t(end - i)});
    }

    // Free temp vertex data
    { std::vector<uint32_t>{}.swap(tmp_qx_); }
    { std::vector<uint32_t>{}.swap(tmp_qy_); }
    { std::vector<uint32_t>{}.swap(tmp_qz_); }
    { std::vector<SortEntry>{}.swap(sorted); }

    // Step 3: Pack face indices into BitPackedArray
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
