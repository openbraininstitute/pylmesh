#pragma once
#include <vector>
#include <cstdint>
#include <array>
#include <limits>
#include <algorithm>

namespace pylmesh
{
class CompressedMesh
{
public:
    using Index = uint32_t;

    struct AABB
    {
        float min[3];
        float max[3];
    };

    // Compressed storage (SoA)
    std::vector<uint16_t> xs, ys, zs;
    std::vector<Index> indices;

    AABB bbox;
    float scale[3];

    bool deltaEncoded = false;

    void clear();
    size_t vertexCount() const;
    size_t faceCount() const;

    // Templates must remain in header
    template<typename VertexArray, typename IndexArray>
    void compress(const VertexArray& vertices, const IndexArray& inds)
    {
        computeBoundingBox(vertices);

        size_t n = vertices.size();
        xs.resize(n);
        ys.resize(n);
        zs.resize(n);

        for (size_t i = 0; i < n; ++i)
        {
            xs[i] = quantize(vertices[i][0], bbox.min[0], scale[0]);
            ys[i] = quantize(vertices[i][1], bbox.min[1], scale[1]);
            zs[i] = quantize(vertices[i][2], bbox.min[2], scale[2]);
        }

        indices = inds;
    }

    std::array<float, 3> getVertex(size_t i) const;
    void getVertex(size_t i, float& x, float& y, float& z) const;

    void applyDeltaEncoding();
    void decodeDelta();

    double surfaceArea() const;

private:
    template<typename VertexArray>
    void computeBoundingBox(const VertexArray& vertices)
    {
        for (int k = 0; k < 3; ++k)
        {
            bbox.min[k] = std::numeric_limits<float>::max();
            bbox.max[k] = std::numeric_limits<float>::lowest();
        }

        for (const auto& v : vertices)
        {
            for (int k = 0; k < 3; ++k)
            {
                bbox.min[k] = std::min(bbox.min[k], v[k]);
                bbox.max[k] = std::max(bbox.max[k], v[k]);
            }
        }

        for (int k = 0; k < 3; ++k)
        {
            scale[k] = (bbox.max[k] - bbox.min[k]) / 65535.0f;
            if (scale[k] == 0.0f) scale[k] = 1.0f;
        }
    }

    static uint16_t quantize(float v, float min, float scale);
    static float dequantize(uint16_t q, float min, float scale);
};
}
