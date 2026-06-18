#!/usr/bin/env python3
"""Test mesh scaling functions"""

import pylmesh


def test_uniform_scale():
    """Test uniform scaling"""
    mesh = pylmesh.Mesh()
    mesh.vertices = [pylmesh.Vertex()]
    mesh.vertices[0].x, mesh.vertices[0].y, mesh.vertices[0].z = 1.0, 2.0, 3.0

    mesh.scale(2.0)

    assert abs(mesh.vertices[0].x - 2.0) < 1e-6
    assert abs(mesh.vertices[0].y - 4.0) < 1e-6
    assert abs(mesh.vertices[0].z - 6.0) < 1e-6


def test_non_uniform_scale():
    """Test non-uniform scaling"""
    mesh = pylmesh.Mesh()
    mesh.vertices = [pylmesh.Vertex()]
    mesh.vertices[0].x, mesh.vertices[0].y, mesh.vertices[0].z = 1.0, 2.0, 3.0

    mesh.scale(2.0, 3.0, 0.5)

    assert abs(mesh.vertices[0].x - 2.0) < 1e-6
    assert abs(mesh.vertices[0].y - 6.0) < 1e-6
    assert abs(mesh.vertices[0].z - 1.5) < 1e-6


def test_scale_preserves_area_ratio():
    """Test that uniform scaling squares the surface area"""
    mesh = pylmesh.Mesh()
    mesh.vertices = [pylmesh.Vertex(), pylmesh.Vertex(), pylmesh.Vertex()]
    mesh.vertices[0].x, mesh.vertices[0].y, mesh.vertices[0].z = 0.0, 0.0, 0.0
    mesh.vertices[1].x, mesh.vertices[1].y, mesh.vertices[1].z = 1.0, 0.0, 0.0
    mesh.vertices[2].x, mesh.vertices[2].y, mesh.vertices[2].z = 0.0, 1.0, 0.0

    face = pylmesh.Face()
    face.indices = [0, 1, 2]
    mesh.faces = [face]

    area_before = mesh.surface_area()
    mesh.scale(3.0)
    area_after = mesh.surface_area()

    assert abs(area_after - area_before * 9.0) < 1e-6


def test_scale_zero():
    """Test scaling by zero"""
    mesh = pylmesh.Mesh()
    mesh.vertices = [pylmesh.Vertex()]
    mesh.vertices[0].x, mesh.vertices[0].y, mesh.vertices[0].z = 5.0, 10.0, 15.0

    mesh.scale(0.0)

    assert mesh.vertices[0].x == 0.0
    assert mesh.vertices[0].y == 0.0
    assert mesh.vertices[0].z == 0.0
