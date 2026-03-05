# pylmesh Notebooks

Jupyter notebooks demonstrating pylmesh usage.

## Available Notebooks

### obj_to_glb.ipynb
Convert OBJ mesh files to GLB (binary GLTF) format with Draco compression.

**Features:**
- Load OBJ files
- Inspect mesh data (vertices, faces)
- Export to GLB format with Draco compression
- Batch conversion of multiple files
- Export to other formats (STL, PLY, OFF, GLTF)

### glb_to_obj.ipynb
Convert GLB (binary GLTF) files to OBJ format with Draco decompression.

**Features:**
- Load GLB files (with Draco decompression)
- Detect Draco compression
- Inspect mesh data
- Export to OBJ format
- Batch conversion
- File size comparison

### visualize_glb.ipynb
Visualize GLB mesh files interactively.

**Features:**
- Load GLB files with Draco decompression
- Convert to trimesh
- Interactive 3D visualization with `.show()`
- Static matplotlib plots (multiple views)
- Mesh statistics (volume, area, watertight, etc.)
- Export rendered images
- Compare multiple meshes

### mesh_conversion.ipynb
General mesh format conversion with error handling.

**Features:**
- Load any supported format
- Convert between formats
- Error handling and diagnostics
- Create test meshes

## Usage

### Install Jupyter
```bash
pip install jupyter
```

### Run Notebook
```bash
cd notebooks
jupyter notebook obj_to_glb.ipynb
```

### Or use JupyterLab
```bash
pip install jupyterlab
jupyter lab
```

## Requirements

```bash
pip install pylmesh jupyter
```

## Example Data

Create a simple test OBJ file:
```python
with open("test.obj", "w") as f:
    f.write("v 0 0 0\n")
    f.write("v 1 0 0\n")
    f.write("v 0 1 0\n")
    f.write("f 1 2 3\n")
```

Then run the notebook to convert it to GLB!
