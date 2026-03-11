#include <lmesh/loader.h>
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <mesh_file>\n";
        return 1;
    }

    pylmesh::Mesh mesh;
    if (pylmesh::MeshLoaderFactory::loadMesh(argv[1], mesh)) {
        std::cout << "Loaded mesh with " << mesh.vertexCount() 
                  << " vertices and " << mesh.faceCount() << " faces\n";
        return 0;
    }

    std::cerr << "Failed to load mesh\n";
    return 1;
}
