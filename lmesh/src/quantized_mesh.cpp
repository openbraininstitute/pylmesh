#include <algorithm>
#include <cmath>
#include <stdexcept>

#include "lmesh/quantized_mesh.h"

namespace pylmesh
{

// ============================================================================
//  QuantizedMesh
// ============================================================================

QuantizedMesh::QuantizedMesh(Vertex min, Vertex max, AxisBits bits)
{
    // Validate per-axis ranges.
    const int axes[3] = {bits.x, bits.y, bits.z};
    for (int i = 0; i < 3; ++i)
    {
        if (axes[i] < 1 || axes[i] > 21)
            throw std::invalid_argument("each axis bits must be in [1, 21]");
    }
    // Total must stay ≤ 63 so bit 63 is always 0 in valid packed values,
    // preserving UINT64_MAX as the FlatHashMap empty-slot sentinel.
    if (bits.total() > 63)
        throw std::invalid_argument("bx + by + bz must be <= 63");

    const float* mn = &min.x;
    const float* mx = &max.x;

    bits_[0]   = bits.x;
    bits_[1]   = bits.y;
    bits_[2]   = bits.z;
    masks_[0]  = (1ull << bits.x) - 1u;
    masks_[1]  = (1ull << bits.y) - 1u;
    masks_[2]  = (1ull << bits.z) - 1u;
    shifts_[0] = 0u;
    shifts_[1] = static_cast<unsigned>(bits.x);
    shifts_[2] = static_cast<unsigned>(bits.x + bits.y);

    for (int i = 0; i < 3; ++i)
    {
        bbox_min_[i]      = mn[i];
        const float range = mx[i] - mn[i];
        bbox_scale_[i]    = range > 0.f ? range / float(masks_[i]) : 1.f;
    }
}

uint64_t QuantizedMesh::encode(float x, float y, float z) const noexcept
{
    // q(val, axis) maps a world coordinate to its grid index, clamped.
    auto q = [&](float val, int axis) -> uint64_t
    {
        float n = (val - bbox_min_[axis]) / bbox_scale_[axis];
        n = std::clamp(n, 0.f, float(masks_[axis]));
        return static_cast<uint64_t>(n + 0.5f);
    };

    // packed = qx | (qy << bx) | (qz << (bx+by))
    return q(x, 0) | (q(y, 1) << shifts_[1]) | (q(z, 2) << shifts_[2]);
}

Vertex QuantizedMesh::decode(uint64_t packed) const noexcept
{
    return {
        bbox_min_[0] + float( packed                & masks_[0]) * bbox_scale_[0],
        bbox_min_[1] + float((packed >> shifts_[1]) & masks_[1]) * bbox_scale_[1],
        bbox_min_[2] + float((packed >> shifts_[2]) & masks_[2]) * bbox_scale_[2]
    };
}

Vertex QuantizedMesh::get_vertex(uint32_t i) const
{
    if (i >= vertex_count())
        throw std::out_of_range("QuantizedMesh::get_vertex: index out of range");
    return decode(vdata_.get(i));
}

QuantizedMesh::Face QuantizedMesh::get_face(uint32_t i) const
{
    if (i >= face_count())
        throw std::out_of_range("QuantizedMesh::get_face: index out of range");
    const size_t base = static_cast<size_t>(i) * 3;
    return {
        static_cast<uint32_t>(indices_.get(base    )),
        static_cast<uint32_t>(indices_.get(base + 1)),
        static_cast<uint32_t>(indices_.get(base + 2))
    };
}

double QuantizedMesh::surface_area() const
{
    double area = 0.0;
    for (uint32_t i = 0, n = face_count(); i < n; ++i)
    {
        const size_t base = static_cast<size_t>(i) * 3;
        const Vertex v0   = decode(vdata_.get(indices_.get(base    )));
        const Vertex v1   = decode(vdata_.get(indices_.get(base + 1)));
        const Vertex v2   = decode(vdata_.get(indices_.get(base + 2)));

        const double ax = v1.x - v0.x, ay = v1.y - v0.y, az = v1.z - v0.z;
        const double bx = v2.x - v0.x, by = v2.y - v0.y, bz = v2.z - v0.z;
        const double cx = ay*bz - az*by, cy = az*bx - ax*bz, cz = ax*by - ay*bx;

        area += std::sqrt(cx*cx + cy*cy + cz*cz);
    }
    return area * 0.5;
}


// ============================================================================
//  QuantizedMeshBuilder
// ============================================================================

uint32_t QuantizedMeshBuilder::index_width(uint32_t n) noexcept
{
    if (n <= 2) return 1;
    uint32_t bits = 0, v = n - 1;
    while (v) { v >>= 1; ++bits; }
    return bits;
}

QuantizedMeshBuilder::QuantizedMeshBuilder(Vertex min, Vertex max,
                                            AxisBits bits, bool dedup)
    : mesh_(min, max, bits)
    , dedup_(dedup)
{}

void QuantizedMeshBuilder::reserve(size_t vertex_count, size_t face_count)
{
    tmp_vdata_.reserve(vertex_count);
    tmp_indices_.reserve(face_count * 3);
    if (dedup_)
        dedup_map_.reserve(vertex_count);
}

uint32_t QuantizedMeshBuilder::add_vertex(float x, float y, float z)
{
    const uint64_t packed = mesh_.encode(x, y, z);

    if (dedup_)
    {
        auto [value, inserted] = dedup_map_.emplace(
            packed, static_cast<uint32_t>(tmp_vdata_.size()));
        if (!inserted)
            return value;
    }

    const uint32_t id = static_cast<uint32_t>(tmp_vdata_.size());
    tmp_vdata_.push_back(packed);
    return id;
}

void QuantizedMeshBuilder::add_face(uint32_t a, uint32_t b, uint32_t c)
{
    tmp_indices_.push_back(a);
    tmp_indices_.push_back(b);
    tmp_indices_.push_back(c);
}

QuantizedMesh QuantizedMeshBuilder::build() &&
{
    // Step 1: free the dedup map.
    dedup_map_.clear_and_free();

    const size_t vcount = tmp_vdata_.size();
    const size_t icount = tmp_indices_.size();

    // Step 2: pack vertices to (bx+by+bz) bits each.
    {
        const uint32_t vwidth = static_cast<uint32_t>(mesh_.vertex_bit_width());
        mesh_.vdata_ = BitPackedArray(vwidth, vcount);
        for (size_t i = 0; i < vcount; ++i)
            mesh_.vdata_.set(i, tmp_vdata_[i]);

        std::vector<uint64_t>{}.swap(tmp_vdata_);
    }

    // Step 3: pack face indices to ⌈log₂(vertex_count)⌉ bits each.
    {
        const uint32_t iwidth = index_width(static_cast<uint32_t>(vcount));
        mesh_.indices_ = BitPackedArray(iwidth, icount);
        for (size_t i = 0; i < icount; ++i)
            mesh_.indices_.set(i, tmp_indices_[i]);

        std::vector<uint32_t>{}.swap(tmp_indices_);
    }

    return std::move(mesh_);
}

} // namespace pylmesh