#pragma once

// ============================================================================
//  UltraCompressedMesh + UltraCompressedMeshBuilder
//
//  Pipeline: quantize → Morton-sort → delta-encode → varint → zstd per chunk
//
//  Sealed-mesh memory:
//    Vertices : ~0.5–2 B/vertex (delta-varint + zstd compressed chunks)
//    Faces    : ⌈log₂(N)⌉ × 3 bits/face (bit-packed)
//    Cache    : fixed 16 × CHUNK_SIZE × 12 B ≈ 1.5 MB (flat array, no heap)
//
//  Lifecycle:
//    UltraCompressedMeshBuilder b(bmin, bmax, 16, false);
//    b.reserve(N, F);
//    for (...) b.add_vertex(x, y, z);
//    for (...) b.add_face(a, b, c);
//    UltraCompressedMesh mesh = std::move(b).build();
// ============================================================================

#include <array>
#include <cstdint>
#include <vector>

#include "lmesh/bit_packed_array.h"
#include "lmesh/flat_hash_map.h"
#include "lmesh/vertex.h"

namespace pylmesh
{

// ── Quantizer ───────────────────────────────────────────────────────────────

struct Quantizer
{
    float min[3] = {};
    float scale[3] = {1.f, 1.f, 1.f};
    int   bits = 16;

    void init(const float* bmin, const float* bmax, int b = 16);
    void quantize(float x, float y, float z,
                  uint32_t& qx, uint32_t& qy, uint32_t& qz) const;
    void dequantize(uint32_t qx, uint32_t qy, uint32_t qz,
                    float& x, float& y, float& z) const;
};

// ── Compressed chunk ────────────────────────────────────────────────────────

struct CompressedChunk
{
    std::vector<uint8_t> data;  // zstd-compressed (or raw varint if no zstd)
    uint32_t count = 0;
};

// ── Flat LRU cache (no heap allocation per access) ──────────────────────────

class FlatLRUCache
{
  public:
    static constexpr uint32_t CAPACITY = 16;

    FlatLRUCache() { for (auto& s : slots_) s.chunk_id = INVALID; }

    // Returns pointer to cached decompressed data, or nullptr on miss.
    uint32_t* get(uint32_t chunk_id, uint32_t chunk_vertex_count) noexcept;

    // Insert decompressed data. Evicts LRU slot.
    uint32_t* put(uint32_t chunk_id, uint32_t chunk_vertex_count,
                  const uint32_t* data);

  private:
    static constexpr uint32_t INVALID = ~uint32_t(0);

    struct Slot
    {
        uint32_t chunk_id = INVALID;
        uint32_t age = 0;
        std::vector<uint32_t> data;  // qx,qy,qz per vertex
    };

    Slot slots_[CAPACITY];
    uint32_t tick_ = 0;
};

// ============================================================================
//  UltraCompressedMesh — sealed, read-only
// ============================================================================

class UltraCompressedMesh
{
  public:
    using Face = std::array<uint32_t, 3>;
    static constexpr uint32_t CHUNK_SIZE = 8192;

    UltraCompressedMesh() = default;

    [[nodiscard]] Vertex get_vertex(uint32_t i);
    [[nodiscard]] Face   get_face(uint32_t i) const;

    uint32_t vertex_count() const noexcept { return vertex_count_; }
    uint32_t face_count()   const noexcept { return static_cast<uint32_t>(indices_.count() / 3); }

    size_t vertex_bytes() const noexcept;
    size_t face_bytes()   const noexcept { return indices_.bytes_used(); }
    size_t total_bytes()  const noexcept { return vertex_bytes() + face_bytes(); }

    [[nodiscard]] double surface_area();

  private:
    friend class UltraCompressedMeshBuilder;

    void decompress_chunk(uint32_t chunk_id, uint32_t* out) const;

    Quantizer Q_;
    std::vector<CompressedChunk> vertex_chunks_;
    uint32_t vertex_count_ = 0;

    BitPackedArray indices_;
    mutable FlatLRUCache cache_;
};

// ============================================================================
//  UltraCompressedMeshBuilder
// ============================================================================

class UltraCompressedMeshBuilder
{
  public:
    explicit UltraCompressedMeshBuilder(Vertex min, Vertex max,
                                        int bits = 16, bool dedup = true);

    void     reserve(size_t vertex_count, size_t face_count);
    uint32_t add_vertex(float x, float y, float z);
    void     add_face(uint32_t a, uint32_t b, uint32_t c);

    uint32_t vertex_count() const noexcept
    {
        return static_cast<uint32_t>(tmp_qx_.size());
    }

    [[nodiscard]] UltraCompressedMesh build() &&;

  private:
    Quantizer Q_;  // reused across all add_vertex calls

    std::vector<uint32_t> tmp_qx_, tmp_qy_, tmp_qz_;
    std::vector<uint32_t> tmp_indices_;

    bool        dedup_;
    FlatHashMap dedup_map_;

    static uint32_t index_width(uint32_t n) noexcept;
};

} // namespace pylmesh
