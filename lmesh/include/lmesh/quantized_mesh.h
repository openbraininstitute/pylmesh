#pragma once

// ============================================================================
//  QuantizedMesh  +  QuantizedMeshBuilder
//
//  Each vertex is packed as:  qx | (qy << bx) | (qz << (bx+by))
//
//  Vertex bit width = bx + by + bz  (each axis independently sized)
//
//  Example configurations:
//
//   x/y/z bits   vertex bits   use-case
//   16/16/16         48        general mesh          (default)
//   16/16/ 8         40        flat terrain, maps    (−17% vs default)
//   10/10/10         30        coarse preview mesh   (−38% vs default)
//    8/ 8/ 4         20        highly compressed     (−58% vs default)
//
//  Face storage: ⌈log₂(vertex_count)⌉ bits per index (unchanged).
//
//  Constraint: bx,by,bz ∈ [1,21] and bx+by+bz ≤ 63.
//  The ≤63 limit keeps bit 63 clear in all valid packed values,
//  preserving UINT64_MAX as the FlatHashMap sentinel.
//
//  Lifecycle
//  ---------
//    AxisBits bits{16, 16, 8};                  // or AxisBits::uniform(16)
//    QuantizedMeshBuilder b(bmin, bmax, bits);
//    b.reserve(verts.size(), faces.size());
//    for (auto& v : verts) b.add_vertex(v.x, v.y, v.z);
//    for (auto& f : faces) b.add_face(f[0], f[1], f[2]);
//    QuantizedMesh mesh = std::move(b).build();
// ============================================================================

#include <array>
#include <cstdint>
#include <vector>

#include "lmesh/base_mesh.h"
#include "lmesh/bit_packed_array.h"
#include "lmesh/flat_hash_map.h"
#include "vertex.h"

namespace pylmesh
{

// ============================================================================
//  AxisBits — per-axis quantization resolution
// ============================================================================
struct AxisBits
{
    int x = 16, y = 16, z = 16;

    // Convenience: all three axes use the same resolution.
    static AxisBits uniform(int bits) noexcept
    {
        return {bits, bits, bits};
    }

    int total() const noexcept
    {
        return x + y + z;
    }
};

// ============================================================================
//  QuantizedMesh — sealed, read-only storage
// ============================================================================
class QuantizedMesh : public BaseMesh
{
  public:
    using Face = std::array<uint32_t, 3>;

    QuantizedMesh() = default;

    // BaseMesh interface
    Vertex   get_vertex(uint32_t i) const override;
    Face     get_face(uint32_t i)   const override;
    uint32_t vertex_count()         const noexcept override;
    uint32_t face_count()           const noexcept override;
    double   surface_area()         const override;
    size_t   vertex_bytes()         const noexcept override;
    size_t   face_bytes()           const noexcept override;
    size_t   total_bytes()          const noexcept override;

    // QuantizedMesh-specific
    AxisBits bits_per_axis() const noexcept
    {
        return {bits_[0], bits_[1], bits_[2]};
    }
    int vertex_bit_width() const noexcept
    {
        return bits_[0] + bits_[1] + bits_[2];
    }

  private:
    friend class QuantizedMeshBuilder;

    QuantizedMesh(Vertex min, Vertex max, AxisBits bits);

    [[nodiscard]] uint64_t encode(float x, float y, float z) const noexcept;
    [[nodiscard]] Vertex decode(uint64_t packed) const noexcept;

    // Per-axis configuration, computed once in the constructor.
    int bits_[3] = {};        // resolution per axis
    uint64_t masks_[3] = {};  // (1ull << bits_[i]) - 1
    unsigned shifts_[3] = {}; // bit offsets: 0,  bx,  bx+by
    float bbox_min_[3] = {};
    float bbox_scale_[3] = {}; // world units per grid step, per axis

    BitPackedArray vdata_;   // width = bx+by+bz,              count = vertex_count
    BitPackedArray indices_; // width = ⌈log₂(vertex_count)⌉,  count = face_count*3
};

// ============================================================================
//  QuantizedMeshBuilder
// ============================================================================
class QuantizedMeshBuilder
{
  public:
    explicit QuantizedMeshBuilder(Vertex min, Vertex max, AxisBits bits = {}, bool dedup = true);

    void reserve(size_t vertex_count, size_t face_count);
    uint32_t add_vertex(float x, float y, float z);
    void add_face(uint32_t a, uint32_t b, uint32_t c);

    uint32_t vertex_count() const noexcept
    {
        return static_cast<uint32_t>(tmp_vdata_.size());
    }

    [[nodiscard]] QuantizedMesh build() &&;

  private:
    QuantizedMesh mesh_;

    std::vector<uint64_t> tmp_vdata_;
    std::vector<uint32_t> tmp_indices_;

    bool dedup_;
    FlatHashMap dedup_map_;

    static uint32_t index_width(uint32_t n) noexcept;
};

} // namespace pylmesh