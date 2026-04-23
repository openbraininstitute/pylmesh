import sys
import os
import psutil
from time import perf_counter
import pylmesh as pl


def benchmark_pylmesh(path):
    mesh = pl.load_quantized_mesh(path)

    if mesh is None:
        raise RuntimeError(f"Failed to load mesh: {path}")

    return mesh


def get_memory_mb():
    process = psutil.Process(os.getpid())
    return process.memory_info().rss / (1024 ** 2)


def main():
    if len(sys.argv) < 2:
        print("Usage: python script.py <mesh_path>")
        sys.exit(1)

    path = sys.argv[1]

    if not os.path.exists(path):
        print(f"File not found: {path}")
        sys.exit(1)

    mem_before = get_memory_mb()

    # ---- Load ----
    t0 = perf_counter()
    mesh = benchmark_pylmesh(path)
    t1 = perf_counter()

    # ---- Surface area ----
    t2 = perf_counter()
    try:
        area = mesh.surface_area()
    except Exception as e:
        print(f"Surface area computation failed: {e}")
        sys.exit(1)
    t3 = perf_counter()

    mem_after = get_memory_mb()

    # ---- Safe attribute access ----
    n_vertices = mesh.vertex_count()
    n_faces = mesh.face_count()

    print(type(mesh))
    print("Vertices:", n_vertices, "Faces:", n_faces)
    print(f"Area: {area:.4f} units²")
    print(f"Time: Loading: {t1 - t0:.4f}s, Area: {t3 - t2:.4f}s")
    print(f"Memory: {mem_before:.2f} -> {mem_after:.2f} MB "
          f"(+{mem_after - mem_before:.2f} MB)")


if __name__ == "__main__":
    main()