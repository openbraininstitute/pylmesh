"""pylmesh - A lightweight library for loading 3D mesh files"""

from ._pylmesh import Mesh, Vertex, Normal, TexCoord, Face, load_mesh, save_mesh
import numpy as np

__version__ = "1.0.0"
__all__ = ["Mesh", "Vertex", "Normal", "TexCoord", "Face", "load_mesh", "save_mesh", "to_trimesh"]

def to_trimesh(mesh):
    """Convert pylmesh.Mesh to trimesh.Trimesh
    
    Args:
        mesh: pylmesh.Mesh object
        
    Returns:
        trimesh.Trimesh object
    """
    try:
        import trimesh
    except ImportError:
        raise ImportError("trimesh is required. Install with: pip install trimesh")
    
    # Get vertices as numpy array (N, 3)
    vertices_flat = mesh.get_vertices_array()
    vertices = np.array(vertices_flat).reshape(-1, 3)
    
    # Get faces as numpy array (M, 3)
    faces_flat = mesh.get_faces_array()
    faces = np.array(faces_flat).reshape(-1, 3)
    
    return trimesh.Trimesh(vertices=vertices, faces=faces)
