# pylmesh API Reference

## Python API

### Loading Meshes

```python
import pylmesh

# Load mesh from file (auto-detects format)
mesh = pylmesh.load_mesh("model.obj")
```

**Supported import formats:**
- `.obj` - Wavefront OBJ
- `.stl` - STL (ASCII)
- `.ply` - PLY (ASCII)
- `.off` - Object File Format
- `.gltf` - GLTF
- `.glb` - GLB (binary GLTF)

### Saving Meshes

```python
# Save mesh to file (auto-detects format from extension)
pylmesh.save_mesh("output.obj", mesh)
pylmesh.save_mesh("output.stl", mesh)
pylmesh.save_mesh("output.ply", mesh)
pylmesh.save_mesh("output.off", mesh)
```

**Supported export formats:**
- `.obj` - Wavefront OBJ
- `.stl` - STL (ASCII)
- `.ply` - PLY (ASCII)
- `.off` - Object File Format
- `.gltf` - GLTF
- `.glb` - GLB (binary GLTF)

### Mesh Class

```python
mesh = pylmesh.Mesh()

# Properties
mesh.vertices      # List of Vertex objects
mesh.normals       # List of Normal objects
mesh.texcoords     # List of TexCoord objects
mesh.faces         # List of Face objects

# Methods
mesh.vertex_count()  # Returns number of vertices
mesh.face_count()    # Returns number of faces
mesh.is_empty()      # Returns True if mesh has no vertices
mesh.clear()         # Clears all mesh data

# Get data as numpy arrays
mesh.get_vertices_array()  # Returns flat list [x1,y1,z1,x2,y2,z2,...]
mesh.get_faces_array()     # Returns flat list [i1,i2,i3,i4,i5,i6,...]
```

### Data Structures

```python
# Vertex
v = pylmesh.Vertex()
v.x = 1.0
v.y = 2.0
v.z = 3.0

# Normal
n = pylmesh.Normal()
n.nx = 0.0
n.ny = 1.0
n.nz = 0.0

# Texture Coordinate
t = pylmesh.TexCoord()
t.u = 0.5
t.v = 0.5

# Face
f = pylmesh.Face()
f.indices = [0, 1, 2]  # Vertex indices
```

### Complete Example

```python
import pylmesh

# Create a triangle mesh
mesh = pylmesh.Mesh()

# Add vertices
v1 = pylmesh.Vertex()
v1.x, v1.y, v1.z = 0.0, 0.0, 0.0

v2 = pylmesh.Vertex()
v2.x, v2.y, v2.z = 1.0, 0.0, 0.0

v3 = pylmesh.Vertex()
v3.x, v3.y, v3.z = 0.5, 1.0, 0.0

mesh.vertices = [v1, v2, v3]

# Add face
face = pylmesh.Face()
face.indices = [0, 1, 2]
mesh.faces = [face]

# Save in different formats
pylmesh.save_mesh("triangle.obj", mesh)
pylmesh.save_mesh("triangle.stl", mesh)
pylmesh.save_mesh("triangle.ply", mesh)
pylmesh.save_mesh("triangle.off", mesh)

# Load and verify
loaded = pylmesh.load_mesh("triangle.obj")
print(f"Vertices: {loaded.vertex_count()}")
print(f"Faces: {loaded.face_count()}")
```

### Convert to Trimesh

```python
import pylmesh
import numpy as np

# Load mesh
mesh = pylmesh.load_mesh("model.glb")

# Get as numpy arrays
vertices = np.array(mesh.get_vertices_array()).reshape(-1, 3)
faces = np.array(mesh.get_faces_array()).reshape(-1, 3)

# Or convert directly to trimesh
tm = pylmesh.to_trimesh(mesh)
print(f"Trimesh: {len(tm.vertices)}v, {len(tm.faces)}f")
```

## C++ API

### Loading Meshes

```cpp
#include <pylmesh/loader.h>

pylmesh::Mesh mesh;
if (pylmesh::MeshLoaderFactory::loadMesh("model.obj", mesh)) {
    // Mesh loaded successfully
}
```

### Saving Meshes

```cpp
#include <pylmesh/exporter.h>

pylmesh::Mesh mesh;
// ... populate mesh ...

if (pylmesh::MeshExporterFactory::saveMesh("output.obj", mesh)) {
    // Mesh saved successfully
}
```

### Mesh Structure

```cpp
pylmesh::Mesh mesh;

// Access data
for (const auto& vertex : mesh.vertices) {
    float x = vertex.x;
    float y = vertex.y;
    float z = vertex.z;
}

for (const auto& face : mesh.faces) {
    for (auto idx : face.indices) {
        // Process vertex index
    }
}

// Methods
size_t vcount = mesh.vertex_count();
size_t fcount = mesh.face_count();
bool empty = mesh.isEmpty();
mesh.clear();
```
