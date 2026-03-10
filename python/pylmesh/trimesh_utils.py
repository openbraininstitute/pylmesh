##########################################################################################
# -*- coding: utf-8 -*-
##########################################################################################
# Copyright (c) 2025 - 2026, Open Brain Institute
#
# Author(s):
#   Marwan Abdellah <marwan.abdellah@openbraininstitute.org>
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
##########################################################################################

"""Trimesh conversion utilities"""

import numpy as np


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
