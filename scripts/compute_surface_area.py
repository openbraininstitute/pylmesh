####################################################################################################
# Copyright (c) 2026
# Open Brain Institute <https://www.openbraininstitute.org/>
#
# For complete list of authors, please see AUTHORS.md
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software distributed under
# the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
####################################################################################################

"""
Tutorial: Computing Surface Area with pylmesh

This example demonstrates how to compute the surface area of 3D meshes.
"""

import pylmesh

# Example 1: Load a mesh and compute its surface area
print("Example 1: Computing surface area from a file")
print("-" * 50)

# Load a mesh from file (supports .obj, .stl, .ply, .off, .gltf, .glb)
mesh = pylmesh.load_mesh("../data/meshes/sphere.glb")

# Compute surface area
area = mesh.surface_area()

print(f"Mesh statistics:")
print(f"  Vertices: {mesh.vertex_count()}")
print(f"  Faces: {mesh.face_count()}")
print(f"  Surface Area: {area:.6f} square units")


# Example 2: Create a simple triangle and compute its area
print("\n\nExample 2: Creating a triangle mesh")
print("-" * 50)

# Create a right triangle with base=1, height=1
# Expected area = 0.5
triangle_mesh = pylmesh.Mesh()
triangle_mesh.vertices = [pylmesh.Vertex(), pylmesh.Vertex(), pylmesh.Vertex()]
triangle_mesh.vertices[0].x, triangle_mesh.vertices[0].y, triangle_mesh.vertices[0].z = 0.0, 0.0, 0.0
triangle_mesh.vertices[1].x, triangle_mesh.vertices[1].y, triangle_mesh.vertices[1].z = 1.0, 0.0, 0.0
triangle_mesh.vertices[2].x, triangle_mesh.vertices[2].y, triangle_mesh.vertices[2].z = 0.0, 1.0, 0.0

triangle_mesh.add_face([0, 1, 2])

area = triangle_mesh.surface_area()
print(f"Triangle area: {area:.6f} (expected: 0.5)")


# Example 3: Create a unit cube and compute its area
print("\n\nExample 3: Creating a unit cube")
print("-" * 50)

# Create a 1x1x1 cube (6 faces, each with area 1)
# Expected total area = 6.0
cube_mesh = pylmesh.Mesh()
cube_mesh.vertices = [pylmesh.Vertex() for _ in range(8)]

# Define 8 vertices of a unit cube
vertices_data = [
    (0, 0, 0), (1, 0, 0), (1, 1, 0), (0, 1, 0),  # bottom face
    (0, 0, 1), (1, 0, 1), (1, 1, 1), (0, 1, 1)   # top face
]

for i, (x, y, z) in enumerate(vertices_data):
    cube_mesh.vertices[i].x = x
    cube_mesh.vertices[i].y = y
    cube_mesh.vertices[i].z = z

# Define 12 triangular faces (2 per cube face)
faces_data = [
    [0, 2, 1], [0, 3, 2],  # bottom (z=0)
    [4, 5, 6], [4, 6, 7],  # top (z=1)
    [0, 1, 5], [0, 5, 4],  # front (y=0)
    [2, 3, 7], [2, 7, 6],  # back (y=1)
    [0, 4, 7], [0, 7, 3],  # left (x=0)
    [1, 2, 6], [1, 6, 5]   # right (x=1)
]

for indices in faces_data:
    cube_mesh.add_face(indices)

area = cube_mesh.surface_area()
print(f"Cube surface area: {area:.6f} (expected: 6.0)")

# Save the cube for visualization
pylmesh.save_mesh("cube.obj", cube_mesh)
print("\nCube saved to 'cube.obj'")


# Example 4: Compare areas of different formats
print("\n\nExample 4: Surface area is preserved across formats")
print("-" * 50)

# Save cube in different formats
formats = ["cube.obj", "cube.stl", "cube.ply", "cube.off", "cube.glb"]
for filename in formats:
    pylmesh.save_mesh(filename, cube_mesh)
    loaded = pylmesh.load_mesh(filename)
    area = loaded.surface_area()
    print(f"{filename:15s} -> Surface area: {area:.6f}")

print("\n✓ Tutorial complete!")
