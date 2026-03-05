"""pylmesh - A lightweight library for loading 3D mesh files"""

from ._pylmesh import Mesh, Vertex, Normal, TexCoord, Face, load_mesh

__version__ = "1.0.0"
__all__ = ["Mesh", "Vertex", "Normal", "TexCoord", "Face", "load_mesh"]
