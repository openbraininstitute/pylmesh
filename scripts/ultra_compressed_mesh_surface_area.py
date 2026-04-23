import sys
import os
import psutil
from time import time

def benchmark_pylmesh(path):
    import pylmesh as pl
    ucmesh = pl.load_ultra_compressed_mesh(path)
    print(ucmesh.vertex_count())
    return ucmesh

def main():
    if len(sys.argv) < 2:
        print("Usage: python script.py <mesh_path>")
        sys.exit(1)

    path = sys.argv[1]

    process = psutil.Process(os.getpid())

    mem_before = process.memory_info().rss / 1024**2

    loading_start = time()
    ucmesh = benchmark_pylmesh(path)
    loading_end = time()

    area_start = time()
    area = ucmesh.surface_area()
    area_end = time()
    mem_after = process.memory_info().rss / 1024**2
    print(f"Memory {mem_before:.2f} -> {mem_after:.2f} MB: Required: {mem_after - mem_before:.2f} MB")
    print(f"Time: Loading: {loading_end - loading_start:.4f} seconds, Area: {area_end - area_start:.4f} seconds")
    print(type(ucmesh))

if __name__ == "__main__":
    main()





