# Installation Guide

## C++ Library

```bash
mkdir build && cd build
cmake ..
make
sudo make install
```

## Python Package

### From source:
```bash
pip install .
```

### Development mode:
```bash
pip install -e .
```

### Build wheel:
```bash
pip install build
python -m build
pip install dist/*.whl
```

### Requirements:
```bash
pip install scikit-build-core pybind11
```

## Usage

### Python:
```python
import pylmesh

mesh = pylmesh.load_mesh("model.obj")
print(f"Vertices: {mesh.vertex_count()}")
print(f"Faces: {mesh.face_count()}")
```

### C++:
```cpp
#include <pylmesh/loader.h>

pylmesh::Mesh mesh;
pylmesh::MeshLoaderFactory::loadMesh("model.obj", mesh);
```
