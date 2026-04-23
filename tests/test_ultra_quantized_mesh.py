import pytest
import pylmesh
import numpy as np
import tempfile
import os


def _make_vertex(x, y, z):
    v = pylmesh.Vertex()
    v.x, v.y, v.z = x, y, z
    return v


def _make_unit_triangle_ucmesh(bits=16):
    b = pylmesh.UltraQuantizedMeshBuilder(
        _make_vertex(0, 0, 0), _make_vertex(1, 1, 0), bits=bits
    )
    s0 = b.add_vertex(0.0, 0.0, 0.0)
    s1 = b.add_vertex(1.0, 0.0, 0.0)
    s2 = b.add_vertex(0.0, 1.0, 0.0)
    b.add_face(s0, s1, s2)
    return b.build()


# ── Construction via builder ─────────────────────────────────────────────────


def test_builder_basic():
    b = pylmesh.UltraQuantizedMeshBuilder(
        _make_vertex(0, 0, 0), _make_vertex(1, 1, 1)
    )
    assert b.vertex_count() == 0


def test_builder_add_and_build():
    b = pylmesh.UltraQuantizedMeshBuilder(
        _make_vertex(0, 0, 0), _make_vertex(10, 10, 10), bits=16
    )
    b.add_vertex(5.0, 5.0, 5.0)
    b.add_face(0, 0, 0)
    ucm = b.build()

    assert ucm.vertex_count() == 1
    v = ucm.get_vertex(0)
    assert abs(v.x - 5.0) < 0.01
    assert abs(v.y - 5.0) < 0.01
    assert abs(v.z - 5.0) < 0.01


def test_builder_dedup():
    b = pylmesh.UltraQuantizedMeshBuilder(
        _make_vertex(0, 0, 0), _make_vertex(1, 1, 1),
        bits=10, dedup=True
    )
    s0 = b.add_vertex(0.5, 0.5, 0.5)
    s1 = b.add_vertex(0.5, 0.5, 0.5)
    ucm = b.build()

    assert s0 == s1
    assert ucm.vertex_count() == 1


def test_builder_no_dedup():
    b = pylmesh.UltraQuantizedMeshBuilder(
        _make_vertex(0, 0, 0), _make_vertex(1, 1, 1),
        bits=10, dedup=False
    )
    b.add_vertex(0.5, 0.5, 0.5)
    b.add_vertex(0.5, 0.5, 0.5)
    ucm = b.build()

    assert ucm.vertex_count() == 2


# ── Vertex / face access ────────────────────────────────────────────────────


def test_get_face():
    ucm = _make_unit_triangle_ucmesh()
    assert ucm.face_count() == 1
    f = ucm.get_face(0)
    assert len(f) == 3
    assert set(f) == {0, 1, 2}


def test_get_face_out_of_range():
    ucm = _make_unit_triangle_ucmesh()
    with pytest.raises((IndexError, RuntimeError)):
        ucm.get_face(999)


def test_multiple_faces():
    b = pylmesh.UltraQuantizedMeshBuilder(
        _make_vertex(0, 0, 0), _make_vertex(1, 1, 0), bits=12
    )
    s0 = b.add_vertex(0.0, 0.0, 0.0)
    s1 = b.add_vertex(1.0, 0.0, 0.0)
    s2 = b.add_vertex(1.0, 1.0, 0.0)
    s3 = b.add_vertex(0.0, 1.0, 0.0)
    b.add_face(s0, s1, s2)
    b.add_face(s0, s2, s3)
    ucm = b.build()

    assert ucm.face_count() == 2
    assert ucm.vertex_count() == 4


# ── Flat arrays ──────────────────────────────────────────────────────────────


def test_get_vertices_array():
    ucm = _make_unit_triangle_ucmesh(bits=16)
    arr = ucm.get_vertices_array()

    assert len(arr) == 9
    verts = np.array(arr).reshape(-1, 3)
    assert verts.shape == (3, 3)


def test_get_faces_array():
    ucm = _make_unit_triangle_ucmesh()
    arr = ucm.get_faces_array()

    assert len(arr) == 3
    assert set(arr) == {0, 1, 2}


# ── Surface area ─────────────────────────────────────────────────────────────


def test_surface_area_unit_triangle():
    ucm = _make_unit_triangle_ucmesh(bits=16)
    assert abs(ucm.surface_area() - 0.5) < 0.01


def test_surface_area_unit_square():
    b = pylmesh.UltraQuantizedMeshBuilder(
        _make_vertex(0, 0, 0), _make_vertex(1, 1, 0), bits=16
    )
    s0 = b.add_vertex(0.0, 0.0, 0.0)
    s1 = b.add_vertex(1.0, 0.0, 0.0)
    s2 = b.add_vertex(1.0, 1.0, 0.0)
    s3 = b.add_vertex(0.0, 1.0, 0.0)
    b.add_face(s0, s1, s2)
    b.add_face(s0, s2, s3)
    ucm = b.build()

    assert abs(ucm.surface_area() - 1.0) < 0.01


# ── Memory introspection ────────────────────────────────────────────────────


def test_memory_bytes():
    ucm = _make_unit_triangle_ucmesh(bits=16)
    assert ucm.vertex_bytes() > 0
    assert ucm.face_bytes() > 0
    assert ucm.total_bytes() == ucm.vertex_bytes() + ucm.face_bytes()


def test_compressed_smaller_than_raw():
    """Vertex storage should be smaller than raw 12 bytes/vertex."""
    b = pylmesh.UltraQuantizedMeshBuilder(
        _make_vertex(0, 0, 0), _make_vertex(100, 100, 100), bits=16
    )
    for i in range(1000):
        b.add_vertex(float(i) * 0.1, float(i) * 0.1, float(i) * 0.1)
    for i in range(0, 999, 3):
        b.add_face(i, min(i+1, 999), min(i+2, 999))
    ucm = b.build()

    raw_vertex_bytes = ucm.vertex_count() * 12
    assert ucm.vertex_bytes() < raw_vertex_bytes


# ── Precision vs bits ────────────────────────────────────────────────────────


def test_higher_bits_better_precision():
    errors = []
    for bits in [8, 12, 16]:
        ucm = _make_unit_triangle_ucmesh(bits=bits)
        errors.append(abs(ucm.surface_area() - 0.5))

    assert errors[-1] <= errors[0]


# ── File roundtrip ───────────────────────────────────────────────────────────


@pytest.mark.parametrize("ext", [".obj", ".stl", ".ply", ".off"])
def test_save_load_roundtrip(ext):
    ucm = _make_unit_triangle_ucmesh(bits=16)

    with tempfile.TemporaryDirectory() as tmpdir:
        filepath = os.path.join(tmpdir, f"test{ext}")
        pylmesh.save_ultra_quantized_mesh(filepath, ucm)
        assert os.path.exists(filepath)

        loaded = pylmesh.load_ultra_quantized_mesh(filepath)
        assert loaded.vertex_count() >= 3
        assert loaded.face_count() >= 1


@pytest.mark.parametrize("ext", [".obj", ".stl", ".ply", ".off"])
def test_roundtrip_surface_area(ext):
    ucm = _make_unit_triangle_ucmesh(bits=16)
    original_area = ucm.surface_area()

    with tempfile.TemporaryDirectory() as tmpdir:
        filepath = os.path.join(tmpdir, f"test{ext}")
        pylmesh.save_ultra_quantized_mesh(filepath, ucm)
        loaded = pylmesh.load_ultra_quantized_mesh(filepath)

        assert abs(loaded.surface_area() - original_area) < 0.05


# ── Load from test_data ──────────────────────────────────────────────────────


TEST_DATA = os.path.join(os.path.dirname(__file__), "test_data")


@pytest.mark.skipif(
    not os.path.exists(os.path.join(TEST_DATA, "test.obj")),
    reason="test_data not available",
)
def test_load_obj_from_test_data():
    ucm = pylmesh.load_ultra_quantized_mesh(os.path.join(TEST_DATA, "test.obj"))
    assert ucm.vertex_count() > 0
    assert ucm.face_count() > 0
    assert ucm.surface_area() > 0.0


# ── Compare with Mesh surface area ──────────────────────────────────────────


@pytest.mark.skipif(
    not os.path.exists(os.path.join(TEST_DATA, "test.obj")),
    reason="test_data not available",
)
def test_surface_area_matches_mesh():
    mesh = pylmesh.load_mesh(os.path.join(TEST_DATA, "test.obj"))
    ucm = pylmesh.load_ultra_quantized_mesh(os.path.join(TEST_DATA, "test.obj"))

    mesh_area = mesh.surface_area()
    ucm_area = ucm.surface_area()

    # Allow 1% tolerance due to quantization
    assert abs(ucm_area - mesh_area) / max(mesh_area, 1e-10) < 0.01
