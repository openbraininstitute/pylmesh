#!/usr/bin/env python3
"""Test surface area calculation"""

import pylmesh


def test_empty_mesh():
    """Test surface area of empty mesh"""
    mesh = pylmesh.Mesh()
    area = mesh.surface_area()
    assert area == 0.0, f"Expected 0.0, got {area}"


def test_triangle():
    """Test surface area of a single triangle"""
    mesh = pylmesh.Mesh()
    mesh.vertices = [pylmesh.Vertex(), pylmesh.Vertex(), pylmesh.Vertex()]
    mesh.vertices[0].x, mesh.vertices[0].y, mesh.vertices[0].z = 0.0, 0.0, 0.0
    mesh.vertices[1].x, mesh.vertices[1].y, mesh.vertices[1].z = 1.0, 0.0, 0.0
    mesh.vertices[2].x, mesh.vertices[2].y, mesh.vertices[2].z = 0.0, 1.0, 0.0
    
    mesh.add_face([0, 1, 2])
    
    area = mesh.surface_area()
    expected = 0.5
    assert abs(area - expected) < 1e-6, f"Expected {expected}, got {area}"


def test_square():
    """Test surface area of a square (two triangles)"""
    mesh = pylmesh.Mesh()
    mesh.vertices = [pylmesh.Vertex() for _ in range(4)]
    mesh.vertices[0].x, mesh.vertices[0].y, mesh.vertices[0].z = 0.0, 0.0, 0.0
    mesh.vertices[1].x, mesh.vertices[1].y, mesh.vertices[1].z = 1.0, 0.0, 0.0
    mesh.vertices[2].x, mesh.vertices[2].y, mesh.vertices[2].z = 1.0, 1.0, 0.0
    mesh.vertices[3].x, mesh.vertices[3].y, mesh.vertices[3].z = 0.0, 1.0, 0.0
    
    mesh.add_face([0, 1, 2])
    mesh.add_face([0, 2, 3])
    
    area = mesh.surface_area()
    expected = 1.0
    assert abs(area - expected) < 1e-6, f"Expected {expected}, got {area}"
