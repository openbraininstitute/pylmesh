#pragma once

// ============================================================================
//  UltraCompressedMesh + UltraCompressedMeshBuilder
//
//  Vertices are quantized, Morton-sorted, delta-encoded, and varint-compressed
//  into fixed-size chunks. An LRU cache decompresses chunks on demand.
//
//  Face indices are stored in a BitPackedArray (same as QuantizedMesh).
//
//  Memory (sealed mesh, after build()):
//    Vertices : ~1–3 B/vertex (delta-varint compressed)
//    Faces    : ⌈log₂(N)⌉ × 3 bits/face (bit-packed)
//
//  Lifecycle:
//    UltraCompressedMeshBuilder b(bmin, bmax);
//    for (...) b.add_vertex(x, y, z);
//    for (...) b.add_face(a, b, c);
//    UltraCompressedMesh mesh = std::move(b).build();
// ============================================================================

#include <array>
#include <cstdint>
#include <list>
#include <unordered_map>
#include <vector>

#include "lmesh/bit_packed_array.h"
#include "lmesh/flat_hash_map.h"
#include "lmesh/vertex.h"

namespace pylmesh
{

// ── Helpers ─────────────────────────────────────────────────────────────────

struct Quantizer
{
    float min[3] = {};
    float scale[3] = {1.f, 1.f, 1.f};

    void init(const float* bmin, const float* bmax, int bits = 16);
    void quantize(float x, float y, float z,
                  uint32_t& qx, uint32_t& qy, uint32_t& qz) const;
    void dequantize(uint32_t qx, uint32_t qy, uint32_t qz,
                    float& x, float& y, float& z) const;
};

struct CompressedChunk
{
    std::vector<uint8_t> data;
    uint32_t count = 0;
};

struct DecompressedChunk
{
    std::vector<uint32_t> vertices; // qx,qy,qz, qx,qy,qz, ...
};

class LRUCache
{
  public:
    explicit LRUCache(size_t capacity = 16);

    bool get(uint32_t key, DecompressedChunk*& value);
    void put(uint32_t key, DecompressedChunk&& value);

  private:
    size_t capacity_;
    std::list<uint32_t> order_;
    std::unordered_map<uint32_t,
        std::pair<std::list<uint32_t>::iterator, DecompressedChunk>> map_;
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

    DecompressedChunk decompress_chunk(uint32_t chunk_id);

    Quantizer Q_;
    std::vector<CompressedChunk> vertex_chunks_;
    uint32_t vertex_count_ = 0;

    BitPackedArray indices_;
    LRUCache cache_{16};
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
    Vertex bmin_, bmax_;
    int bits_;

    // Temporary storage during construction
    std::vector<uint32_t> tmp_qx_, tmp_qy_, tmp_qz_;
    std::vector<uint32_t> tmp_indices_;

    bool        dedup_;
    FlatHashMap dedup_map_;

    static uint32_t index_width(uint32_t n) noexcept;
};

} // namespace pylmesh
