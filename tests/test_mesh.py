import pytest
import pylmesh
import numpy as np


def test_mesh_creation():
    """Test creating an empty mesh"""
    mesh = pylmesh.Mesh()
    assert mesh.is_empty()
    assert mesh.vertex_count() == 0
    assert mesh.face_count() == 0


def test_mesh_vertices():
    """Test adding vertices to mesh"""
    mesh = pylmesh.Mesh()
    
    v1 = pylmesh.Vertex()
    v1.x, v1.y, v1.z = 0.0, 0.0, 0.0
    
    v2 = pylmesh.Vertex()
    v2.x, v2.y, v2.z = 1.0, 0.0, 0.0
    
    v3 = pylmesh.Vertex()
    v3.x, v3.y, v3.z = 0.0, 1.0, 0.0
    
    mesh.vertices = [v1, v2, v3]
    
    assert mesh.vertex_count() == 3
    assert not mesh.is_empty()


def test_mesh_faces():
    """Test adding faces to mesh"""
    mesh = pylmesh.Mesh()
    mesh.vertices = [pylmesh.Vertex() for _ in range(3)]
    
    mesh.add_face([0, 1, 2])
    
    assert mesh.face_count() == 1
    assert mesh.get_face_indices(0) == [0, 1, 2]


def test_mesh_clear():
    """Test clearing mesh data"""
    mesh = pylmesh.Mesh()
    mesh.vertices = [pylmesh.Vertex() for _ in range(3)]
    
    assert not mesh.is_empty()
    
    mesh.clear()
    
    assert mesh.is_empty()
    assert mesh.vertex_count() == 0


def test_get_vertices_array():
    """Test getting vertices as flat array"""
    mesh = pylmesh.Mesh()
    
    v1 = pylmesh.Vertex()
    v1.x, v1.y, v1.z = 1.0, 2.0, 3.0
    
    v2 = pylmesh.Vertex()
    v2.x, v2.y, v2.z = 4.0, 5.0, 6.0
    
    mesh.vertices = [v1, v2]
    
    arr = mesh.get_vertices_array()
    assert len(arr) == 6
    assert arr == [1.0, 2.0, 3.0, 4.0, 5.0, 6.0]


def test_get_faces_array():
    """Test getting faces as flat array"""
    mesh = pylmesh.Mesh()
    mesh.vertices = [pylmesh.Vertex() for _ in range(6)]
    
    mesh.add_face([0, 1, 2])
    mesh.add_face([3, 4, 5])
    
    arr = mesh.get_faces_array()
    assert len(arr) == 6
    assert arr == [0, 1, 2, 3, 4, 5]
