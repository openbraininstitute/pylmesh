/*****************************************************************************************
 * Copyright (c) 2025 - 2026, Open Brain Institute
 *
 * Author(s):
 *   Marwan Abdellah <marwan.abdellah@openbraininstitute.org>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under
 * the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *****************************************************************************************/

#include "lmesh/exporter.h"
#include "lmesh/loader.h"
#include "lmesh/mesh.h"
#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/vector.h>

namespace nb = nanobind;

NB_MODULE(_pylmesh, m)
{
    m.doc() = "Python bindings for pylmesh - 3D mesh file loader";

    nb::class_<pylmesh::Vertex>(m, "Vertex")
        .def(nb::init<>())
        .def_rw("x", &pylmesh::Vertex::x)
        .def_rw("y", &pylmesh::Vertex::y)
        .def_rw("z", &pylmesh::Vertex::z);

    nb::class_<pylmesh::Normal>(m, "Normal")
        .def(nb::init<>())
        .def_rw("nx", &pylmesh::Normal::nx)
        .def_rw("ny", &pylmesh::Normal::ny)
        .def_rw("nz", &pylmesh::Normal::nz);

    nb::class_<pylmesh::TexCoord>(m, "TexCoord")
        .def(nb::init<>())
        .def_rw("u", &pylmesh::TexCoord::u)
        .def_rw("v", &pylmesh::TexCoord::v);

    nb::class_<pylmesh::Face>(m, "Face")
        .def(nb::init<>())
        .def_rw("indices", &pylmesh::Face::indices);

    nb::class_<pylmesh::Mesh>(m, "Mesh")
        .def(nb::init<>())
        .def_rw("vertices", &pylmesh::Mesh::vertices)
        .def_rw("normals", &pylmesh::Mesh::normals)
        .def_rw("texcoords", &pylmesh::Mesh::texcoords)
        .def_rw("faces", &pylmesh::Mesh::faces)
        .def("clear", &pylmesh::Mesh::clear)
        .def("is_empty", &pylmesh::Mesh::isEmpty)
        .def("vertex_count", &pylmesh::Mesh::vertexCount)
        .def("face_count", &pylmesh::Mesh::faceCount)
        .def("get_vertices_array", &pylmesh::Mesh::getVerticesArray)
        .def("get_faces_array", &pylmesh::Mesh::getFacesArray)
        .def("surface_area", &pylmesh::Mesh::surfaceArea);

    m.def(
        "load_mesh",
        [](const std::string& filepath)
        {
            pylmesh::Mesh mesh;
            if (pylmesh::MeshLoaderFactory::loadMesh(filepath, mesh))
            {
                return mesh;
            }
            throw std::runtime_error("Failed to load mesh: " + filepath +
                                     ". Check if file exists and format is supported.");
        },
        nb::arg("filepath"),
        "Load a mesh from file (supports .obj, .stl, .ply, .off, .gltf, .glb)");

    m.def(
        "save_mesh",
        [](const std::string& filepath, const pylmesh::Mesh& mesh)
        {
            if (pylmesh::MeshExporterFactory::saveMesh(filepath, mesh))
            {
                return true;
            }
            throw std::runtime_error("Failed to save mesh: " + filepath);
        },
        nb::arg("filepath"), nb::arg("mesh"),
        "Save a mesh to file (supports .obj, .stl, .ply, .off)");
}
