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

"""pylmesh - A lightweight library for importing and exporting surface mesh files"""

from ._pylmesh import Mesh, Vertex, Normal, TexCoord, load_mesh, save_mesh
from .trimesh_utils import to_trimesh
from .batch_conversion_glb import batch_convert_to_glb
from pathlib import Path

try:
    version_file = Path(__file__).parent.parent.parent / "pylmesh.version"
    if version_file.exists():
        __version__ = version_file.read_text().strip()
    else:
        import importlib.metadata
        __version__ = importlib.metadata.version("pylmesh")
except Exception:
    __version__ = "0.0.0.dev"

__all__ = ["Mesh", 
           "Vertex", 
           "Normal", 
           "TexCoord", 
           "load_mesh", 
           "save_mesh", 
           "to_trimesh", 
           "batch_convert_to_glb"]
