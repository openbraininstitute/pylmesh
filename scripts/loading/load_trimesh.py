import sys
import os
import psutil
from time import perf_counter
import trimesh


def benchmark_trimesh(path):
    mesh = trimesh.load(path, force='mesh')

    # If it's a scene, merge into a single mesh
    if isinstance(mesh, trimesh.Scene):
        mesh = trimesh.util.concatenate(mesh.dump())

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
    mesh = benchmark_trimesh(path)
    t1 = perf_counter()

    # ---- Area ----
    t2 = perf_counter()
    area = mesh.area
    t3 = perf_counter()

    mem_after = get_memory_mb()

    print(type(mesh))
    print("Vertices:", len(mesh.vertices), "Faces:", len(mesh.faces))
    print(f"Area: {area:.4f} units²")
    print(f"Time: Loading: {t1 - t0:.4f}s, Area: {t3 - t2:.4f}s")
    print(f"Memory: {mem_before:.2f} -> {mem_after:.2f} MB "
          f"(+{mem_after - mem_before:.2f} MB)")


if __name__ == "__main__":
    main()