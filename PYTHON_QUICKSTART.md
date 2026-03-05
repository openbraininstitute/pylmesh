# Python Quick Start

## Installation

```bash
pip install .
```

Or install from wheel:
```bash
pip install dist/pylmesh-1.0.0-cp312-cp312-linux_x86_64.whl
```

## Usage

```python
import pylmesh

# Load a mesh file (supports .obj, .stl, .ply)
mesh = pylmesh.load_mesh("model.obj")

# Get mesh info
print(f"Vertices: {mesh.vertex_count()}")
print(f"Faces: {mesh.face_count()}")

# Access vertices
for vertex in mesh.vertices:
    print(f"Vertex: ({vertex.x}, {vertex.y}, {vertex.z})")

# Access faces
for face in mesh.faces:
    print(f"Face indices: {face.indices}")

# Access normals (if available)
for normal in mesh.normals:
    print(f"Normal: ({normal.nx}, {normal.ny}, {normal.nz})")

# Clear mesh
mesh.clear()
```

## Build Wheel

```bash
pip install build
python -m build --wheel
```

## Development Install

```bash
pip install -e .
```
