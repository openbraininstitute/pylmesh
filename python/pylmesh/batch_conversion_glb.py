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

"""Batch conversion utilities for GLB format"""

from pathlib import Path
from multiprocessing import cpu_count
from joblib import Parallel, delayed
from . import load_mesh, save_mesh


def batch_convert_to_glb(input_path, output_dir, workers=None):
    """Convert OBJ meshes to GLB in parallel.
    
    Args:
        input_path: Path to OBJ file or directory containing OBJ files (str or Path)
        output_dir: Directory where GLB files will be written (str or Path)
        workers: Number of worker processes (default: cpu_count - 1)
        
    Returns:
        dict with keys:
            - 'success': number of successful conversions
            - 'failure': number of failed conversions
            - 'successful_pairs': list of (obj_path, glb_path) tuples
            - 'errors': list of (obj_path, error_message) tuples
    """
    input_path = Path(input_path)
    output_dir = Path(output_dir)
    
    if workers is None:
        workers = max(1, cpu_count() - 1)
    
    if not input_path.exists():
        raise FileNotFoundError(f"Input path does not exist: {input_path}")
    
    if input_path.is_file():
        if input_path.suffix.lower() != ".obj":
            raise ValueError(f"Input file is not an OBJ file: {input_path}")
        obj_files = [input_path]
    else:
        obj_files = sorted(p for p in input_path.iterdir() 
                          if p.is_file() and p.suffix.lower() == ".obj")
    
    if not obj_files:
        raise ValueError(f"No OBJ files found in: {input_path}")
    
    output_dir.mkdir(parents=True, exist_ok=True)
    
    def convert_single(obj_path):
        try:
            mesh = load_mesh(str(obj_path))
            glb_path = output_dir / f"{obj_path.stem}.glb"
            save_mesh(str(glb_path), mesh)
            return obj_path, glb_path, None
        except Exception as exc:
            return obj_path, None, str(exc)
    
    results = Parallel(n_jobs=workers, backend="loky")(
        delayed(convert_single)(obj_path) for obj_path in obj_files
    )
    
    successful_pairs = []
    errors = []
    
    for obj_path, glb_path, error in results:
        if error is None and glb_path is not None:
            successful_pairs.append((obj_path, glb_path))
        else:
            errors.append((obj_path, error))
    
    return {
        'success': len(successful_pairs),
        'failure': len(errors),
        'successful_pairs': successful_pairs,
        'errors': errors
    }
