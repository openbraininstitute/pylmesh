#!/usr/bin/env python3
"""Test all import/export formats"""

import pylmesh
import pytest
import tempfile
import os


def test_obj_format():
    """Test OBJ format import/export"""
    mesh = pylmesh.Mesh()
    mesh.vertices = [pylmesh.Vertex(), pylmesh.Vertex(), pylmesh.Vertex()]
    mesh.vertices[0].x, mesh.vertices[0].y, mesh.vertices[0].z = 0.0, 0.0, 0.0
    mesh.vertices[1].x, mesh.vertices[1].y, mesh.vertices[1].z = 1.0, 0.0, 0.0
    mesh.vertices[2].x, mesh.vertices[2].y, mesh.vertices[2].z = 0.5, 1.0, 0.0
    
    face = pylmesh.Face()
    face.indices = [0, 1, 2]
    mesh.faces = [face]
    
    with tempfile.TemporaryDirectory() as tmpdir:
        filepath = os.path.join(tmpdir, "test.obj")
        pylmesh.save_mesh(filepath, mesh)
        loaded = pylmesh.load_mesh(filepath)
        
        assert loaded.vertex_count() == 3
        assert loaded.face_count() == 1


def test_stl_format():
    """Test STL format import/export"""
    mesh = pylmesh.Mesh()
    mesh.vertices = [pylmesh.Vertex(), pylmesh.Vertex(), pylmesh.Vertex()]
    mesh.vertices[0].x, mesh.vertices[0].y, mesh.vertices[0].z = 0.0, 0.0, 0.0
    mesh.vertices[1].x, mesh.vertices[1].y, mesh.vertices[1].z = 1.0, 0.0, 0.0
    mesh.vertices[2].x, mesh.vertices[2].y, mesh.vertices[2].z = 0.5, 1.0, 0.0
    
    face = pylmesh.Face()
    face.indices = [0, 1, 2]
    mesh.faces = [face]
    
    with tempfile.TemporaryDirectory() as tmpdir:
        filepath = os.path.join(tmpdir, "test.stl")
        pylmesh.save_mesh(filepath, mesh)
        loaded = pylmesh.load_mesh(filepath)
        
        assert loaded.vertex_count() == 3
        assert loaded.face_count() == 1


def test_ply_format():
    """Test PLY format import/export"""
    mesh = pylmesh.Mesh()
    mesh.vertices = [pylmesh.Vertex(), pylmesh.Vertex(), pylmesh.Vertex()]
    mesh.vertices[0].x, mesh.vertices[0].y, mesh.vertices[0].z = 0.0, 0.0, 0.0
    mesh.vertices[1].x, mesh.vertices[1].y, mesh.vertices[1].z = 1.0, 0.0, 0.0
    mesh.vertices[2].x, mesh.vertices[2].y, mesh.vertices[2].z = 0.5, 1.0, 0.0
    
    face = pylmesh.Face()
    face.indices = [0, 1, 2]
    mesh.faces = [face]
    
    with tempfile.TemporaryDirectory() as tmpdir:
        filepath = os.path.join(tmpdir, "test.ply")
        pylmesh.save_mesh(filepath, mesh)
        loaded = pylmesh.load_mesh(filepath)
        
        assert loaded.vertex_count() == 3
        assert loaded.face_count() == 1


def test_off_format():
    """Test OFF format import/export"""
    mesh = pylmesh.Mesh()
    mesh.vertices = [pylmesh.Vertex(), pylmesh.Vertex(), pylmesh.Vertex()]
    mesh.vertices[0].x, mesh.vertices[0].y, mesh.vertices[0].z = 0.0, 0.0, 0.0
    mesh.vertices[1].x, mesh.vertices[1].y, mesh.vertices[1].z = 1.0, 0.0, 0.0
    mesh.vertices[2].x, mesh.vertices[2].y, mesh.vertices[2].z = 0.5, 1.0, 0.0
    
    face = pylmesh.Face()
    face.indices = [0, 1, 2]
    mesh.faces = [face]
    
    with tempfile.TemporaryDirectory() as tmpdir:
        filepath = os.path.join(tmpdir, "test.off")
        pylmesh.save_mesh(filepath, mesh)
        loaded = pylmesh.load_mesh(filepath)
        
        assert loaded.vertex_count() == 3
        assert loaded.face_count() == 1
