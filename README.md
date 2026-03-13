<p align="center">
  <img src="docs/images/pylmesh-logo.jpeg" alt="pylmesh logo" width="800"/>
</p>

# pylmesh

pylmesh is a **l**ightweight **python** library for importing and exporting surface **mesh** files in multiple formats. 
The library is designed in C++ with python bindings to feature high performance and loading large scale meshes. 

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

# Compute surface area
area = mesh.surface_area()
print(f"Surface area: {area}")

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

### System Requirements

**macOS:**
```bash
brew install libomp
```

**Linux (Ubuntu/Debian):**
```bash
sudo apt-get install libomp-dev
```

**Linux (Fedora/RHEL):**
```bash
sudo dnf install libomp-devel
```

### Python Package

**Core functionality only (no dependencies):**
```bash
pip install pylmesh
```

**With optional features:**
```bash
# Add development tools
pip install pylmesh[dev]
```

**From source:**
```bash
pip install .
```

### C++ Library
```bash
mkdir build && cd build
cmake ..
make
```

<p align="center">
  Copyright (c) 2025-2026 Open Brain Institute
</p>