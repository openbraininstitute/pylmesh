# pylmesh Notebooks

Jupyter notebooks demonstrating pylmesh usage.

## Available Notebooks

### obj_to_glb.ipynb
Convert OBJ mesh files to GLB (binary GLTF) format.

**Features:**
- Load OBJ files
- Inspect mesh data (vertices, faces)
- Export to GLB format
- Batch conversion of multiple files
- Export to other formats (STL, PLY, OFF, GLTF)

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
