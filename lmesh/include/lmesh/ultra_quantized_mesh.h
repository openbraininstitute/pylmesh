#pragma once

// ============================================================================
//  UltraQuantizedMesh + UltraQuantizedMeshBuilder
//
//  Design priorities: minimum sealed memory, O(chunk) access acceptable.
//
//  Compression pipeline (both vertices AND faces):
//    1. Quantize x/y/z → uint32_t grid coords
//    2. Morton-sort vertices for spatial locality
//    3. Sort faces by min(a,b,c) remapped index → local index deltas
//    4. Per chunk: column-orient streams (all-dx, all-dy, all-dz)
//    5. Delta-encode → zigzag → varint
//    6. zstd level 19 per chunk
//
//  Memory (sealed, excluding LRU cache):
//    Vertices : ~1.8–2.5 B/vertex
//    Faces    : ~3.5–5 B/face
//    Target   : ~70–90 MB for 11.7M vert / 23.5M face mesh
//               vs 250 MB previous, 299 MB QuantizedMesh, 524 MB naive
//
//  LRU cache (decompressed working set, NOT part of sealed mesh):
//    CACHE_SLOTS × CHUNK_SIZE × 3 × 4 B ≈ 25 MB for defaults
//    Trades memory for decompression frequency.
//    Tune CACHE_SLOTS down to reduce memory if access is rare.
//
//  Chunk size tradeoff:
//    Larger → better zstd ratio, higher cache cost, slower random seek
//    CHUNK_SIZE = 32768 is the sweet spot for memory-first use.
//
//  Lifecycle:
//    UltraQuantizedMeshBuilder b(bmin, bmax, /*bits=*/16, /*dedup=*/true);
//    b.reserve(N, F);
//    for (...) b.add_vertex(x, y, z);
//    for (...) b.add_face(a, b, c);
//    UltraQuantizedMesh mesh = std::move(b).build();
// ============================================================================

#include <array>
#include <cstdint>
#include <vector>

#include "lmesh/base_mesh.h"
#include "lmesh/flat_hash_map.h"
#include "lmesh/vertex.h"

namespace pylmesh
{

// ── Quantizer ───────────────────────────────────────────────────────────────

struct Quantizer
{
    float    min[3]   = {};
    float    scale[3] = {1.f, 1.f, 1.f};
    int      bits     = 21;

    void init(const float* bmin, const float* bmax, int b = 16);
    void quantize(float x, float y, float z,
                  uint32_t& qx, uint32_t& qy, uint32_t& qz) const noexcept;
    void dequantize(uint32_t qx, uint32_t qy, uint32_t qz,
                    float& x,   float& y,   float& z)   const noexcept;
};

// ── Compressed chunk ────────────────────────────────────────────────────────

struct CompressedChunk
{
    std::vector<uint8_t> data;  // zstd(varint(delta(column-interleaved coords)))
    uint32_t             count = 0;
};

// ── Flat LRU cache ──────────────────────────────────────────────────────────
//
//  Holds CACHE_SLOTS decompressed chunks. Each slot is a flat uint32_t[]
//  in column order: [qx0..qxN, qy0..qyN, qz0..qzN].
//  Memory: CACHE_SLOTS × CHUNK_SIZE × 3 × 4 B ≈ 25 MB at defaults.

class FlatLRUCache
{
  public:
    static constexpr uint32_t INVALID = ~uint32_t(0);

    explicit FlatLRUCache(uint32_t slots, uint32_t chunk_size);

    // Returns pointer to [qx0..qxN, qy0..qyN, qz0..qzN] for chunk_id,
    // or nullptr on miss.
    const uint32_t* get(uint32_t chunk_id) const noexcept;

    // Evict LRU slot, fill with data[0..count*3), return pointer to it.
    const uint32_t* put(uint32_t chunk_id, uint32_t count,
                        const uint32_t* col_data);

  private:
    struct Slot
    {
        uint32_t chunk_id = INVALID;
        uint32_t age      = 0;
    };

    uint32_t              slots_;
    uint32_t              chunk_size_;
    uint32_t              tick_ = 0;
    std::vector<Slot>     meta_;
    std::vector<uint32_t> data_; // slots_ × chunk_size_ × 3 — one flat allocation
};

// ============================================================================
//  UltraQuantizedMesh — sealed, read-only
// ============================================================================

class UltraQuantizedMesh : public BaseMesh
{
  public:
    static constexpr uint32_t CHUNK_SIZE   = 32768;
    static constexpr uint32_t CACHE_SLOTS  = 64;

    using Face = std::array<uint32_t, 3>;

    UltraQuantizedMesh();

    // BaseMesh interface
    Vertex   get_vertex(uint32_t i) const override;
    Face     get_face(uint32_t i)   const override;
    uint32_t vertex_count()         const noexcept override;
    uint32_t face_count()           const noexcept override;
    double   surface_area()         const override;
    size_t   vertex_bytes()         const noexcept override;
    size_t   face_bytes()           const noexcept override;
    size_t   total_bytes()          const noexcept override;

    // UltraQuantizedMesh-specific
    size_t cache_bytes() const noexcept;

  private:
    friend class UltraQuantizedMeshBuilder;

    // Decompress chunk into column-oriented output:
    //   out[0..count)        = qx values
    //   out[count..2*count)  = qy values
    //   out[2*count..3*count)= qz values
    void decompress_vertex_chunk(uint32_t chunk_id, uint32_t* out) const;
    void decompress_face_chunk  (uint32_t chunk_id, uint32_t* out) const;

    // Returns pointer to decompressed column data for chunk_id.
    // Decompresses and caches on miss.
    const uint32_t* vertex_chunk_data(uint32_t chunk_id) const;
    const uint32_t* face_chunk_data  (uint32_t chunk_id) const;

    Quantizer Q_;

    std::vector<CompressedChunk> vertex_chunks_;
    std::vector<CompressedChunk> face_chunks_;

    uint32_t vertex_count_ = 0;
    uint32_t face_count_   = 0;

    // Mutable: logically const (cache is an implementation detail).
    mutable FlatLRUCache vcache_;
    mutable FlatLRUCache fcache_;
};

// ============================================================================
//  UltraQuantizedMeshBuilder
// ============================================================================

class UltraQuantizedMeshBuilder
{
  public:
    explicit UltraQuantizedMeshBuilder(Vertex min, Vertex max,
                                        int  bits  = 21,
                                        bool dedup = true);

    void     reserve(size_t vertex_count, size_t face_count);
    uint32_t add_vertex(float x, float y, float z);
    void     add_face(uint32_t a, uint32_t b, uint32_t c);

    uint32_t vertex_count() const noexcept
    {
        return static_cast<uint32_t>(tmp_verts_.size());
    }

    [[nodiscard]] UltraQuantizedMesh build() &&;

  private:
    Quantizer Q_;

    // AoS during building: one entry per vertex.
    struct QVert { uint32_t x, y, z; };
    std::vector<QVert>    tmp_verts_;
    std::vector<uint32_t> tmp_indices_; // 3 per face

    bool        dedup_;
    FlatHashMap dedup_map_;

    static uint32_t index_width(uint32_t n) noexcept;
};

} // namespace pylmesh