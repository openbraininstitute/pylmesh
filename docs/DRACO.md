# Draco Compression Support

pylmesh now supports Draco mesh compression for GLB files, significantly reducing file sizes.

## Features

- **Automatic Draco compression** for GLB exports
- **Draco decompression** for GLB imports
- **KHR_draco_mesh_compression** extension support
- Configurable quantization levels

## Installation

Draco is automatically downloaded and linked during build:

```bash
pip install .
```

To disable Draco:
```bash
pip install . --config-settings=cmake.args="-DENABLE_DRACO=OFF"
```

## Usage

### Python

Draco compression is automatically applied when saving GLB files:

```python
import pylmesh

mesh = pylmesh.load_mesh("model.obj")

# Automatically uses Draco compression
pylmesh.save_mesh("output.glb", mesh)
```

### Verification

Check if a GLB file uses Draco compression:

```python
with open("output.glb", "rb") as f:
    data = f.read()
    if b"KHR_draco_mesh_compression" in data:
        print("✓ Draco compressed")
    else:
        print("✗ Uncompressed")
```

## Compression Settings

Current settings (optimized for quality):
- **Speed**: 5/10 (balanced)
- **Position quantization**: 14 bits
- **Extension**: KHR_draco_mesh_compression (required)

## File Size Comparison

Typical compression ratios:

| Format | Size | Compression |
|--------|------|-------------|
| OBJ    | 100% | None        |
| GLB (uncompressed) | ~60% | Binary format |
| GLB (Draco) | ~20% | Draco + binary |

Example with 10,000 vertex mesh:
- OBJ: ~500 KB
- GLB (uncompressed): ~300 KB
- GLB (Draco): ~100 KB

## Compatibility

Draco-compressed GLB files are supported by:
- ✓ Three.js
- ✓ Babylon.js
- ✓ Blender 2.8+
- ✓ Unity (with plugin)
- ✓ Unreal Engine
- ✓ glTF Viewer
- ✓ Most modern 3D viewers

## Build Options

### Enable Draco (default)
```bash
cmake -DENABLE_DRACO=ON ..
```

### Disable Draco
```bash
cmake -DENABLE_DRACO=OFF ..
```

## Technical Details

### Draco Library
- Version: 1.5.7
- License: Apache 2.0
- Source: https://github.com/google/draco

### Implementation
- Encodes vertex positions with 14-bit quantization
- Compresses triangle indices
- Adds KHR_draco_mesh_compression extension
- Marks extension as required in GLTF

### Limitations
- Only triangle meshes supported
- Point clouds not compressed with Draco
- Normals and texcoords not yet compressed (coming soon)
