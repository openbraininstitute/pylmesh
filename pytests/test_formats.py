import pytest
import pylmesh
import tempfile
import os


@pytest.fixture
def sample_mesh():
    """Create a simple triangle mesh"""
    mesh = pylmesh.Mesh()
    
    v1 = pylmesh.Vertex()
    v1.x, v1.y, v1.z = 0.0, 0.0, 0.0
    
    v2 = pylmesh.Vertex()
    v2.x, v2.y, v2.z = 1.0, 0.0, 0.0
    
    v3 = pylmesh.Vertex()
    v3.x, v3.y, v3.z = 0.5, 1.0, 0.0
    
    mesh.vertices = [v1, v2, v3]
    
    face = pylmesh.Face()
    face.indices = [0, 1, 2]
    mesh.faces = [face]
    
    return mesh


@pytest.mark.parametrize("format_ext", [".obj", ".stl", ".ply", ".off", ".gltf", ".glb"])
def test_save_load_roundtrip(sample_mesh, format_ext):
    """Test saving and loading mesh in different formats"""
    with tempfile.TemporaryDirectory() as tmpdir:
        filepath = os.path.join(tmpdir, f"test{format_ext}")
        
        # Save
        pylmesh.save_mesh(filepath, sample_mesh)
        assert os.path.exists(filepath)
        
        # Load
        loaded_mesh = pylmesh.load_mesh(filepath)
        
        # Verify
        assert loaded_mesh.vertex_count() == sample_mesh.vertex_count()
        assert loaded_mesh.face_count() == sample_mesh.face_count()


def test_obj_format(sample_mesh):
    """Test OBJ format specifically"""
    with tempfile.TemporaryDirectory() as tmpdir:
        filepath = os.path.join(tmpdir, "test.obj")
        
        pylmesh.save_mesh(filepath, sample_mesh)
        loaded = pylmesh.load_mesh(filepath)
        
        assert loaded.vertex_count() == 3
        assert loaded.face_count() == 1


def test_glb_format(sample_mesh):
    """Test GLB format with Draco compression"""
    with tempfile.TemporaryDirectory() as tmpdir:
        filepath = os.path.join(tmpdir, "test.glb")
        
        pylmesh.save_mesh(filepath, sample_mesh)
        
        # Check for Draco compression
        with open(filepath, 'rb') as f:
            data = f.read()
            has_draco = b'KHR_draco_mesh_compression' in data
        
        loaded = pylmesh.load_mesh(filepath)
        
        assert loaded.vertex_count() == 3
        assert loaded.face_count() == 1
        assert has_draco  # Should have Draco compression


def test_invalid_file():
    """Test loading non-existent file"""
    with pytest.raises(RuntimeError):
        pylmesh.load_mesh("nonexistent.obj")


def test_unsupported_format(sample_mesh):
    """Test saving to unsupported format"""
    with tempfile.TemporaryDirectory() as tmpdir:
        filepath = os.path.join(tmpdir, "test.xyz")
        
        with pytest.raises(RuntimeError):
            pylmesh.save_mesh(filepath, sample_mesh)
