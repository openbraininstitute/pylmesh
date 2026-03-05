#pragma once
#include <vector>
#include <string>

namespace pylmesh {

struct Vertex {
    float x, y, z;
};

struct Normal {
    float nx, ny, nz;
};

struct TexCoord {
    float u, v;
};

struct Face {
    std::vector<unsigned int> indices;
};

class Mesh {
public:
    std::vector<Vertex> vertices;
    std::vector<Normal> normals;
    std::vector<TexCoord> texcoords;
    std::vector<Face> faces;

    void clear();
    bool isEmpty() const;
    size_t vertexCount() const;
    size_t faceCount() const;
    
    // Get vertices as flat array [x1, y1, z1, x2, y2, z2, ...]
    std::vector<float> getVerticesArray() const;
    
    // Get faces as flat array [i1, i2, i3, i4, i5, i6, ...]
    std::vector<unsigned int> getFacesArray() const;
};

}
