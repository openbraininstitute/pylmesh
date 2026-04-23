import pytest
import pylmesh
import numpy as np
import tempfile
import os


def _make_vertex(x, y, z):
    v = pylmesh.Vertex()
    v.x, v.y, v.z = x, y, z
    return v


def _make_unit_triangle_qmesh(bits=16):
    """Create a QuantizedMesh with a single right triangle in the XY plane."""
    b = pylmesh.QuantizedMeshBuilder(
        _make_vertex(0, 0, 0), _make_vertex(1, 1, 0), bits_per_axis=bits
    )
    s0 = b.add_vertex(0.0, 0.0, 0.0)
    s1 = b.add_vertex(1.0, 0.0, 0.0)
    s2 = b.add_vertex(0.0, 1.0, 0.0)
    b.add_face(s0, s1, s2)
    return b.build()


# ── Construction via builder ─────────────────────────────────────────────────


def test_builder_basic():
    b = pylmesh.QuantizedMeshBuilder(
        _make_vertex(0, 0, 0), _make_vertex(1, 1, 1)
    )
    assert b.vertex_count() == 0


def test_builder_add_and_build():
    b = pylmesh.QuantizedMeshBuilder(
        _make_vertex(0, 0, 0), _make_vertex(10, 10, 10), bits_per_axis=16
    )
    b.add_vertex(5.0, 5.0, 5.0)
    b.add_face(0, 0, 0)
    qm = b.build()

    assert qm.vertex_count() == 1
    v = qm.get_vertex(0)
    assert abs(v.x - 5.0) < 0.01
    assert abs(v.y - 5.0) < 0.01
    assert abs(v.z - 5.0) < 0.01


def test_builder_dedup():
    b = pylmesh.QuantizedMeshBuilder(
        _make_vertex(0, 0, 0), _make_vertex(1, 1, 1),
        bits_per_axis=10, dedup=True
    )
    s0 = b.add_vertex(0.5, 0.5, 0.5)
    s1 = b.add_vertex(0.5, 0.5, 0.5)
    qm = b.build()

    assert s0 == s1
    assert qm.vertex_count() == 1


def test_builder_no_dedup():
    b = pylmesh.QuantizedMeshBuilder(
        _make_vertex(0, 0, 0), _make_vertex(1, 1, 1),
        bits_per_axis=10, dedup=False
    )
    b.add_vertex(0.5, 0.5, 0.5)
    b.add_vertex(0.5, 0.5, 0.5)
    qm = b.build()

    assert qm.vertex_count() == 2


# ── Vertex / face access ────────────────────────────────────────────────────


def test_get_vertex_out_of_range():
    qm = _make_unit_triangle_qmesh()
    with pytest.raises(IndexError):
        qm.get_vertex(999)


def test_get_face():
    qm = _make_unit_triangle_qmesh()
    assert qm.face_count() == 1
    f = qm.get_face(0)
    assert len(f) == 3
    assert set(f) == {0, 1, 2}


def test_get_face_out_of_range():
    qm = _make_unit_triangle_qmesh()
    with pytest.raises(IndexError):
        qm.get_face(999)


def test_multiple_faces():
    b = pylmesh.QuantizedMeshBuilder(
        _make_vertex(0, 0, 0), _make_vertex(1, 1, 0), bits_per_axis=12
    )
    s0 = b.add_vertex(0.0, 0.0, 0.0)
    s1 = b.add_vertex(1.0, 0.0, 0.0)
    s2 = b.add_vertex(1.0, 1.0, 0.0)
    s3 = b.add_vertex(0.0, 1.0, 0.0)
    b.add_face(s0, s1, s2)
    b.add_face(s0, s2, s3)
    qm = b.build()

    assert qm.face_count() == 2
    assert qm.vertex_count() == 4


# ── Flat arrays ──────────────────────────────────────────────────────────────


def test_get_vertices_array():
    qm = _make_unit_triangle_qmesh(bits=16)
    arr = qm.get_vertices_array()

    assert len(arr) == 9
    verts = np.array(arr).reshape(-1, 3)
    assert verts.shape == (3, 3)


def test_get_faces_array():
    qm = _make_unit_triangle_qmesh()
    arr = qm.get_faces_array()

    assert len(arr) == 3
    assert set(arr) == {0, 1, 2}


# ── Surface area ─────────────────────────────────────────────────────────────


def test_surface_area_unit_triangle():
    qm = _make_unit_triangle_qmesh(bits=16)
    assert abs(qm.surface_area() - 0.5) < 0.01


def test_surface_area_unit_square():
    b = pylmesh.QuantizedMeshBuilder(
        _make_vertex(0, 0, 0), _make_vertex(1, 1, 0), bits_per_axis=16
    )
    s0 = b.add_vertex(0.0, 0.0, 0.0)
    s1 = b.add_vertex(1.0, 0.0, 0.0)
    s2 = b.add_vertex(1.0, 1.0, 0.0)
    s3 = b.add_vertex(0.0, 1.0, 0.0)
    b.add_face(s0, s1, s2)
    b.add_face(s0, s2, s3)
    qm = b.build()

    assert abs(qm.surface_area() - 1.0) < 0.01


# ── Memory introspection ────────────────────────────────────────────────────


def test_memory_bytes():
    qm = _make_unit_triangle_qmesh(bits=16)
    assert qm.vertex_bytes() > 0
    assert qm.face_bytes() > 0
    assert qm.total_bytes() == qm.vertex_bytes() + qm.face_bytes()


# ── Precision vs bits ────────────────────────────────────────────────────────


def test_higher_bits_better_precision():
    errors = []
    for bits in [8, 12, 16]:
        qm = _make_unit_triangle_qmesh(bits=bits)
        errors.append(abs(qm.surface_area() - 0.5))

    assert errors[-1] <= errors[0]


# ── File roundtrip ───────────────────────────────────────────────────────────


@pytest.mark.parametrize("ext", [".obj", ".stl", ".ply", ".off"])
def test_save_load_roundtrip(ext):
    qm = _make_unit_triangle_qmesh(bits=16)

    with tempfile.TemporaryDirectory() as tmpdir:
        filepath = os.path.join(tmpdir, f"test{ext}")
        pylmesh.save_quantized_mesh(filepath, qm)
        assert os.path.exists(filepath)

        loaded = pylmesh.load_quantized_mesh(filepath)
        assert loaded.vertex_count() >= 3
        assert loaded.face_count() >= 1


@pytest.mark.parametrize("ext", [".obj", ".stl", ".ply", ".off"])
def test_roundtrip_surface_area(ext):
    qm = _make_unit_triangle_qmesh(bits=16)
    original_area = qm.surface_area()

    with tempfile.TemporaryDirectory() as tmpdir:
        filepath = os.path.join(tmpdir, f"test{ext}")
        pylmesh.save_quantized_mesh(filepath, qm)
        loaded = pylmesh.load_quantized_mesh(filepath)

        assert abs(loaded.surface_area() - original_area) < 0.05


# ── Load from test_data ──────────────────────────────────────────────────────


TEST_DATA = os.path.join(os.path.dirname(__file__), "test_data")


@pytest.mark.skipif(
    not os.path.exists(os.path.join(TEST_DATA, "test.obj")),
    reason="test_data not available",
)
def test_load_obj_from_test_data():
    qm = pylmesh.load_quantized_mesh(os.path.join(TEST_DATA, "test.obj"))
    assert qm.vertex_count() > 0
    assert qm.face_count() > 0
    assert qm.surface_area() > 0.0
