#!/usr/bin/env python3
"""Example usage of pylmesh Python bindings"""

import pylmesh
import sys

def main():
    if len(sys.argv) < 2:
        print(f"Usage: {sys.argv[0]} <mesh_file>")
        return 1

    try:
        mesh = pylmesh.load_mesh(sys.argv[1])
        print(f"Loaded mesh with {mesh.vertex_count()} vertices and {mesh.face_count()} faces")
        
        if mesh.vertex_count() > 0:
            v = mesh.vertices[0]
            print(f"First vertex: ({v.x}, {v.y}, {v.z})")
        
        return 0
    except Exception as e:
        print(f"Error: {e}")
        return 1

if __name__ == "__main__":
    sys.exit(main())
