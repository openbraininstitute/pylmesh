# pylmesh

A lightweight C++ library for loading 3D mesh files in multiple formats.

## Supported Formats
- OBJ (Wavefront)
- STL (ASCII)
- PLY (ASCII)



## Usage

### Python
```python
import pylmesh

mesh = pylmesh.load_mesh("model.obj")
print(f"Vertices: {mesh.vertex_count()}")
print(f"Faces: {mesh.face_count()}")
```

### C++
```cpp
#include <pylmesh/loader.h>

pylmesh::Mesh mesh;
if (pylmesh::MeshLoaderFactory::loadMesh("model.obj", mesh)) {
    // Use mesh data
}
```

## Installation

### Python Package
```bash
pip install .
```

### C++ Library
```bash
mkdir build && cd build
cmake ..
make
```

## Architecture
- `include/pylmesh/` - Public headers
- `src/` - Implementation
- `examples/` - Usage examples
- `tests/` - Unit tests
