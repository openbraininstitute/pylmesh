import sys
import os
import psutil
from time import time

def benchmark_pylmesh(path):
    import pylmesh as pl
    qmesh = pl.load_quantized_mesh(path)
    print(qmesh.vertex_count())
    return qmesh

def main():
    if len(sys.argv) < 2:
        print("Usage: python script.py <mesh_path>")
        sys.exit(1)

    path = sys.argv[1]

    process = psutil.Process(os.getpid())

    mem_before = process.memory_info().rss / 1024**2

    loading_start = time()
    qmesh = benchmark_pylmesh(path)
    loading_end = time()

    area_start = time()
    area = qmesh.surface_area()
    area_end = time()
    mem_after = process.memory_info().rss / 1024**2
    print(f"Memory {mem_before:.2f} -> {mem_after:.2f} MB: Required: {mem_after - mem_before:.2f} MB")
    print(f"Time: Loading: {loading_end - loading_start:.4f} seconds, Area: {area_end - area_start:.4f} seconds")
    print(type(qmesh))

if __name__ == "__main__":
    main()





