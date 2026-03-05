#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "pylmesh/mesh.h"
#include "pylmesh/loader.h"

namespace py = pybind11;

PYBIND11_MODULE(_pylmesh, m) {
    m.doc() = "Python bindings for pylmesh - 3D mesh file loader";

    py::class_<pylmesh::Vertex>(m, "Vertex")
        .def(py::init<>())
        .def_readwrite("x", &pylmesh::Vertex::x)
        .def_readwrite("y", &pylmesh::Vertex::y)
        .def_readwrite("z", &pylmesh::Vertex::z);

    py::class_<pylmesh::Normal>(m, "Normal")
        .def(py::init<>())
        .def_readwrite("nx", &pylmesh::Normal::nx)
        .def_readwrite("ny", &pylmesh::Normal::ny)
        .def_readwrite("nz", &pylmesh::Normal::nz);

    py::class_<pylmesh::TexCoord>(m, "TexCoord")
        .def(py::init<>())
        .def_readwrite("u", &pylmesh::TexCoord::u)
        .def_readwrite("v", &pylmesh::TexCoord::v);

    py::class_<pylmesh::Face>(m, "Face")
        .def(py::init<>())
        .def_readwrite("indices", &pylmesh::Face::indices);

    py::class_<pylmesh::Mesh>(m, "Mesh")
        .def(py::init<>())
        .def_readwrite("vertices", &pylmesh::Mesh::vertices)
        .def_readwrite("normals", &pylmesh::Mesh::normals)
        .def_readwrite("texcoords", &pylmesh::Mesh::texcoords)
        .def_readwrite("faces", &pylmesh::Mesh::faces)
        .def("clear", &pylmesh::Mesh::clear)
        .def("is_empty", &pylmesh::Mesh::isEmpty)
        .def("vertex_count", &pylmesh::Mesh::vertexCount)
        .def("face_count", &pylmesh::Mesh::faceCount);

    m.def("load_mesh", [](const std::string& filepath) {
        pylmesh::Mesh mesh;
        if (pylmesh::MeshLoaderFactory::loadMesh(filepath, mesh)) {
            return mesh;
        }
        throw std::runtime_error("Failed to load mesh: " + filepath);
    }, py::arg("filepath"), "Load a mesh from file");
}
