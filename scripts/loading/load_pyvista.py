import sys
import os
import psutil
from time import perf_counter
import pyvista as pv


def benchmark_pyvista(path):
    mesh = pv.read(path)

    # Handle MultiBlock datasets safely
    if isinstance(mesh, pv.MultiBlock):
        mesh = mesh.combine()

    # Ensure we work with a surface mesh
    if not isinstance(mesh, pv.PolyData):
        mesh = mesh.extract_surface()

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
    mesh = benchmark_pyvista(path)
    t1 = perf_counter()

    # ---- Area ----
    t2 = perf_counter()
    area = mesh.area
    t3 = perf_counter()

    mem_after = get_memory_mb()

    print(type(mesh))
    print("Vertices:", mesh.n_points, "Faces:", mesh.n_cells)
    print(f"Area: {area:.4f} units²")
    print(f"Time: Loading: {t1 - t0:.4f}s, Area: {t3 - t2:.4f}s")
    print(f"Memory: {mem_before:.2f} -> {mem_after:.2f} MB "
          f"(+{mem_after - mem_before:.2f} MB)")


if __name__ == "__main__":
    main()