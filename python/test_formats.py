#!/usr/bin/env python3
"""Test all import/export formats"""

import pylmesh
import sys

def test_format(format_name, input_file, output_file):
    """Test loading and saving a specific format"""
    try:
        mesh = pylmesh.load_mesh(input_file)
        print(f"✓ {format_name} Import: {mesh.vertex_count()} vertices, {mesh.face_count()} faces")
        
        pylmesh.save_mesh(output_file, mesh)
        
        mesh2 = pylmesh.load_mesh(output_file)
        print(f"✓ {format_name} Export: {mesh2.vertex_count()} vertices, {mesh2.face_count()} faces")
        
        return True
    except Exception as e:
        print(f"✗ {format_name} failed: {e}")
        return False

def main():
    print("Testing pylmesh import/export functionality\n")
    
    # Create test mesh
    mesh = pylmesh.Mesh()
    mesh.vertices = [
        pylmesh.Vertex(),
        pylmesh.Vertex(),
        pylmesh.Vertex()
    ]
    mesh.vertices[0].x, mesh.vertices[0].y, mesh.vertices[0].z = 0.0, 0.0, 0.0
    mesh.vertices[1].x, mesh.vertices[1].y, mesh.vertices[1].z = 1.0, 0.0, 0.0
    mesh.vertices[2].x, mesh.vertices[2].y, mesh.vertices[2].z = 0.5, 1.0, 0.0
    
    face = pylmesh.Face()
    face.indices = [0, 1, 2]
    mesh.faces = [face]
    
    # Test all formats
    formats = [
        ("OBJ", "test.obj"),
        ("STL", "test.stl"),
        ("PLY", "test.ply"),
        ("OFF", "test.off")
    ]
    
    results = []
    for name, ext in formats:
        pylmesh.save_mesh(ext, mesh)
        result = test_format(name, ext, f"output_{ext}")
        results.append(result)
    
    print(f"\n{'='*50}")
    print(f"Results: {sum(results)}/{len(results)} formats working")
    print(f"{'='*50}")
    
    print("\nSupported formats:")
    print("  Import: OBJ, STL, PLY, OFF, GLTF/GLB (basic)")
    print("  Export: OBJ, STL, PLY, OFF")
    
    return 0 if all(results) else 1

if __name__ == "__main__":
    sys.exit(main())
