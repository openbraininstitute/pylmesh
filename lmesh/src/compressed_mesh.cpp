#include "lmesh/compressed_mesh.h"
#include <cmath>

namespace pylmesh
{

void CompressedMesh::clear()
{
    xs.clear(); ys.clear(); zs.clear();
    indices.clear();
    deltaEncoded = false;
}

size_t CompressedMesh::vertexCount() const { return xs.size(); }

size_t CompressedMesh::faceCount() const { return indices.size() / 3; }

std::array<float, 3> CompressedMesh::getVertex(size_t i) const
{
    return {
        dequantize(xs[i], bbox.min[0], scale[0]),
        dequantize(ys[i], bbox.min[1], scale[1]),
        dequantize(zs[i], bbox.min[2], scale[2])
    };
}

void CompressedMesh::getVertex(size_t i, float& x, float& y, float& z) const
{
    x = dequantize(xs[i], bbox.min[0], scale[0]);
    y = dequantize(ys[i], bbox.min[1], scale[1]);
    z = dequantize(zs[i], bbox.min[2], scale[2]);
}

void CompressedMesh::applyDeltaEncoding()
{
    for (size_t i = xs.size() - 1; i > 0; --i)
    {
        xs[i] -= xs[i - 1];
        ys[i] -= ys[i - 1];
        zs[i] -= zs[i - 1];
    }
    deltaEncoded = true;
}

void CompressedMesh::decodeDelta()
{
    for (size_t i = 1; i < xs.size(); ++i)
    {
        xs[i] += xs[i - 1];
        ys[i] += ys[i - 1];
        zs[i] += zs[i - 1];
    }
    deltaEncoded = false;
}

uint16_t CompressedMesh::quantize(float v, float min, float scale)
{
    return static_cast<uint16_t>((v - min) / scale);
}

float CompressedMesh::dequantize(uint16_t q, float min, float scale)
{
    return min + q * scale;
}

double CompressedMesh::surfaceArea() const
{
    double area = 0.0;
    const size_t nFaces = faceCount();

    for (size_t f = 0; f < nFaces; ++f)
    {
        auto v0 = getVertex(indices[f * 3]);
        auto v1 = getVertex(indices[f * 3 + 1]);
        auto v2 = getVertex(indices[f * 3 + 2]);

        double ax = v1[0] - v0[0];
        double ay = v1[1] - v0[1];
        double az = v1[2] - v0[2];

        double bx = v2[0] - v0[0];
        double by = v2[1] - v0[1];
        double bz = v2[2] - v0[2];

        double cx = ay * bz - az * by;
        double cy = az * bx - ax * bz;
        double cz = ax * by - ay * bx;

        area += 0.5 * std::sqrt(cx * cx + cy * cy + cz * cz);
    }

    return area;
}

}
