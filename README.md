# pylmesh

A lightweight C++ library for loading 3D mesh files in multiple formats.

## Supported Formats
- OBJ (Wavefront) - Import/Export
- STL (ASCII) - Import/Export
- PLY (ASCII) - Import/Export
- OFF (Object File Format) - Import/Export
- GLTF - Import/Export
- GLB - Import/Export (with Draco compression)



## Usage

### Python
```python
import pylmesh

# Load mesh
mesh = pylmesh.load_mesh("model.obj")
print(f"Vertices: {mesh.vertex_count()}")
print(f"Faces: {mesh.face_count()}")

# Save mesh
pylmesh.save_mesh("output.stl", mesh)
```

### C++
```cpp
#include <pylmesh/loader.h>
#include <pylmesh/exporter.h>

pylmesh::Mesh mesh;
if (pylmesh::MeshLoaderFactory::loadMesh("model.obj", mesh)) {
    // Use mesh data
    pylmesh::MeshExporterFactory::saveMesh("output.stl", mesh);
}
```

## Installation

### Python Package
```bash
pip install .
```

With trimesh support:
```bash
pip install .[trimesh]
```

With development tools:
```bash
pip install .[dev]
```

Dependencies are automatically downloaded during build. No manual setup required!

### C++ Library
```bash
mkdir build && cd build
cmake ..
make
```

All dependencies (tinygltf, nlohmann/json, stb, draco) are automatically downloaded.

## Architecture
- `include/pylmesh/` - Public headers
- `src/` - Implementation
- `examples/` - Usage examples
- `tests/` - Unit tests
