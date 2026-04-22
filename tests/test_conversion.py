import pytest
import pylmesh
import numpy as np


@pytest.fixture
def sample_mesh():
    """Create a simple triangle mesh"""
    mesh = pylmesh.Mesh()
    
    mesh.vertices = [pylmesh.Vertex() for _ in range(3)]
    mesh.vertices[0].x, mesh.vertices[0].y, mesh.vertices[0].z = 0.0, 0.0, 0.0
    mesh.vertices[1].x, mesh.vertices[1].y, mesh.vertices[1].z = 1.0, 0.0, 0.0
    mesh.vertices[2].x, mesh.vertices[2].y, mesh.vertices[2].z = 0.5, 1.0, 0.0
    
    mesh.add_face([0, 1, 2])
    
    return mesh


def test_vertices_to_numpy(sample_mesh):
    """Test converting vertices to numpy array"""
    vertices_flat = sample_mesh.get_vertices_array()
    vertices = np.array(vertices_flat).reshape(-1, 3)
    
    assert vertices.shape == (3, 3)
    assert vertices[0, 0] == 0.0
    assert vertices[1, 0] == 1.0
    assert vertices[2, 0] == 0.5


def test_faces_to_numpy(sample_mesh):
    """Test converting faces to numpy array"""
    faces_flat = sample_mesh.get_faces_array()
    faces = np.array(faces_flat).reshape(-1, 3)
    
    assert faces.shape == (1, 3)
    assert list(faces[0]) == [0, 1, 2]


@pytest.mark.skipif(not pytest.importorskip("trimesh", reason="trimesh not installed"),
                    reason="trimesh not installed")
def test_to_trimesh(sample_mesh):
    """Test converting to trimesh"""
    tm = pylmesh.to_trimesh(sample_mesh)
    
    assert len(tm.vertices) == 3
    assert len(tm.faces) == 1
    assert tm.vertices.shape == (3, 3)
    assert tm.faces.shape == (1, 3)
