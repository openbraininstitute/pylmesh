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
#include "lmesh/quantized_mesh.h"
#include "lmesh/ultra_quantized_mesh.h"
#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/vector.h>
#include <nanobind/stl/array.h>

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

    nb::class_<pylmesh::Mesh>(m, "Mesh")
        .def(nb::init<>())
        .def_rw("vertices", &pylmesh::Mesh::vertices)
        .def_rw("normals", &pylmesh::Mesh::normals)
        .def_rw("texcoords", &pylmesh::Mesh::texcoords)
        .def_rw("indices", &pylmesh::Mesh::indices)
        .def_rw("face_offsets", &pylmesh::Mesh::face_offsets)
        .def("clear", &pylmesh::Mesh::clear)
        .def("is_empty", &pylmesh::Mesh::is_empty)
        .def("vertex_count", &pylmesh::Mesh::vertex_count)
        .def("face_count", &pylmesh::Mesh::face_count)
        .def("get_vertices_array", &pylmesh::Mesh::get_vertices_array)
        .def("get_faces_array", &pylmesh::Mesh::get_faces_array)
        .def("surface_area", &pylmesh::Mesh::surface_area)
        .def("vertex_bytes", &pylmesh::Mesh::vertex_bytes,
             "Bytes used for vertex storage")
        .def("face_bytes", &pylmesh::Mesh::face_bytes,
             "Bytes used for face storage")
        .def("total_bytes", &pylmesh::Mesh::total_bytes,
             "Total bytes used")
        .def("add_face",
             [](pylmesh::Mesh& self, const std::vector<uint32_t>& idx)
             { self.add_face(idx.data(), idx.size()); },
             nb::arg("indices"), "Add a face from a list of vertex indices")
        .def("get_face_indices",
             [](const pylmesh::Mesh& self, size_t f) -> std::vector<uint32_t>
             {
                 if (f >= self.face_count())
                     throw std::out_of_range("Face index out of range");
                 const uint32_t* ptr = self.face_indices(f);
                 return {ptr, ptr + self.face_size(f)};
             },
             nb::arg("face_index"), "Get vertex indices for a face");

    nb::class_<pylmesh::AxisBits>(m, "AxisBits")
        .def(nb::init<int, int, int>(),
             nb::arg("x"), nb::arg("y"), nb::arg("z"))
        .def_static("uniform", &pylmesh::AxisBits::uniform, nb::arg("bits"))
        .def_ro("x", &pylmesh::AxisBits::x)
        .def_ro("y", &pylmesh::AxisBits::y)
        .def_ro("z", &pylmesh::AxisBits::z);

    nb::class_<pylmesh::QuantizedMesh>(m, "QuantizedMesh")
        .def("get_vertex", &pylmesh::QuantizedMesh::get_vertex,
             nb::arg("i"), "Decode vertex i back to float space")
        .def("get_face", &pylmesh::QuantizedMesh::get_face,
             nb::arg("i"), "Decode face i")
        .def("vertex_count", &pylmesh::QuantizedMesh::vertex_count)
        .def("face_count", &pylmesh::QuantizedMesh::face_count)
        .def("bits_per_axis", &pylmesh::QuantizedMesh::bits_per_axis)
        .def("surface_area", &pylmesh::QuantizedMesh::surface_area,
             "Compute total surface area")
        .def("vertex_bytes", &pylmesh::QuantizedMesh::vertex_bytes,
             "Bytes used for vertex storage")
        .def("face_bytes", &pylmesh::QuantizedMesh::face_bytes,
             "Bytes used for face storage")
        .def("total_bytes", &pylmesh::QuantizedMesh::total_bytes,
             "Total bytes used for vertex + face storage")
        .def("get_vertices_array",
             [](const pylmesh::QuantizedMesh& self) -> std::vector<float>
             {
                 std::vector<float> result;
                 result.reserve(self.vertex_count() * 3);
                 for (uint32_t i = 0; i < self.vertex_count(); ++i)
                 {
                     pylmesh::Vertex v = self.get_vertex(i);
                     result.push_back(v.x);
                     result.push_back(v.y);
                     result.push_back(v.z);
                 }
                 return result;
             },
             "Get all vertices as flat array [x1,y1,z1,x2,y2,z2,...]")
        .def("get_faces_array",
             [](const pylmesh::QuantizedMesh& self) -> std::vector<uint32_t>
             {
                 std::vector<uint32_t> result;
                 result.reserve(self.face_count() * 3);
                 for (uint32_t i = 0; i < self.face_count(); ++i)
                 {
                     auto f = self.get_face(i);
                     result.push_back(f[0]);
                     result.push_back(f[1]);
                     result.push_back(f[2]);
                 }
                 return result;
             },
             "Get all faces as flat array [i1,i2,i3,...]");

    nb::class_<pylmesh::QuantizedMeshBuilder>(m, "QuantizedMeshBuilder")
        .def("__init__",
             [](pylmesh::QuantizedMeshBuilder* self,
                pylmesh::Vertex min, pylmesh::Vertex max,
                int bits_per_axis, bool dedup)
             { new (self) pylmesh::QuantizedMeshBuilder(min, max, pylmesh::AxisBits::uniform(bits_per_axis), dedup); },
             nb::arg("min"), nb::arg("max"),
             nb::arg("bits_per_axis") = 16, nb::arg("dedup") = true,
             "Create a builder with bounding box and precision")
        .def("reserve", &pylmesh::QuantizedMeshBuilder::reserve,
             nb::arg("vertex_count"), nb::arg("face_count"),
             "Pre-allocate storage")
        .def("add_vertex", &pylmesh::QuantizedMeshBuilder::add_vertex,
             nb::arg("x"), nb::arg("y"), nb::arg("z"),
             "Quantize and store a vertex, returns slot index")
        .def("add_face", &pylmesh::QuantizedMeshBuilder::add_face,
             nb::arg("a"), nb::arg("b"), nb::arg("c"),
             "Store a triangle face from slot indices")
        .def("vertex_count", &pylmesh::QuantizedMeshBuilder::vertex_count)
        .def("build",
             [](pylmesh::QuantizedMeshBuilder& self) -> pylmesh::QuantizedMesh
             { return std::move(self).build(); },
             "Consume the builder and return a sealed QuantizedMesh");

    m.def(
        "load_mesh",
        [](const std::string& filepath)
        {
            pylmesh::Mesh mesh;
            if (pylmesh::MeshLoaderFactory::load_mesh(filepath, mesh))
            {
                return mesh;
            }
            throw std::runtime_error("Failed to load mesh: " + filepath +
                                     ". Check if file exists and format is supported.");
        },
        nb::arg("filepath"),
        "Load a mesh from file (supports .obj, .stl, .ply, .off, .gltf, .glb)");

    m.def(
        "load_quantized_mesh",
        [](const std::string& filepath)
        {
            pylmesh::QuantizedMesh mesh;
            if (pylmesh::MeshLoaderFactory::load_mesh(filepath, mesh))
            {
                return mesh;
            }
            throw std::runtime_error("Failed to load quantized mesh: " + filepath +
                                     ". Check if file exists and format is supported.");
        },
        nb::arg("filepath"),
        "Load a mesh into a QuantizedMesh (supports .obj, .stl, .ply, .off, .gltf, .glb)");

    m.def(
        "save_mesh",
        [](const std::string& filepath, const pylmesh::Mesh& mesh)
        {
            if (pylmesh::MeshExporterFactory::save_mesh(filepath, mesh))
            {
                return true;
            }
            throw std::runtime_error("Failed to save mesh: " + filepath);
        },
        nb::arg("filepath"), nb::arg("mesh"),
        "Save a mesh to file (supports .obj, .stl, .ply, .off, .gltf, .glb)");

    m.def(
        "save_quantized_mesh",
        [](const std::string& filepath, const pylmesh::QuantizedMesh& mesh)
        {
            if (pylmesh::MeshExporterFactory::save_mesh(filepath, mesh))
            {
                return true;
            }
            throw std::runtime_error("Failed to save quantized mesh: " + filepath);
        },
        nb::arg("filepath"), nb::arg("mesh"),
        "Save a QuantizedMesh to file (supports .obj, .stl, .ply, .off, .gltf, .glb)");

    // ── UltraQuantizedMesh ─────────────────────────────────────────────────

    nb::class_<pylmesh::UltraQuantizedMesh>(m, "UltraQuantizedMesh")
        .def("get_vertex", &pylmesh::UltraQuantizedMesh::get_vertex,
             nb::arg("i"), "Decode vertex i back to float space")
        .def("get_face", &pylmesh::UltraQuantizedMesh::get_face,
             nb::arg("i"), "Decode face i")
        .def("vertex_count", &pylmesh::UltraQuantizedMesh::vertex_count)
        .def("face_count", &pylmesh::UltraQuantizedMesh::face_count)
        .def("surface_area", &pylmesh::UltraQuantizedMesh::surface_area,
             "Compute total surface area")
        .def("vertex_bytes", &pylmesh::UltraQuantizedMesh::vertex_bytes,
             "Bytes used for compressed vertex storage")
        .def("face_bytes", &pylmesh::UltraQuantizedMesh::face_bytes,
             "Bytes used for face storage")
        .def("total_bytes", &pylmesh::UltraQuantizedMesh::total_bytes,
             "Total bytes used")
        .def("get_vertices_array",
             [](pylmesh::UltraQuantizedMesh& self) -> std::vector<float>
             {
                 std::vector<float> result;
                 result.reserve(self.vertex_count() * 3);
                 for (uint32_t i = 0; i < self.vertex_count(); ++i)
                 {
                     pylmesh::Vertex v = self.get_vertex(i);
                     result.push_back(v.x);
                     result.push_back(v.y);
                     result.push_back(v.z);
                 }
                 return result;
             },
             "Get all vertices as flat array [x1,y1,z1,x2,y2,z2,...]")
        .def("get_faces_array",
             [](const pylmesh::UltraQuantizedMesh& self) -> std::vector<uint32_t>
             {
                 std::vector<uint32_t> result;
                 result.reserve(self.face_count() * 3);
                 for (uint32_t i = 0; i < self.face_count(); ++i)
                 {
                     auto f = self.get_face(i);
                     result.push_back(f[0]);
                     result.push_back(f[1]);
                     result.push_back(f[2]);
                 }
                 return result;
             },
             "Get all faces as flat array [i1,i2,i3,...]");

    nb::class_<pylmesh::UltraQuantizedMeshBuilder>(m, "UltraQuantizedMeshBuilder")
        .def(nb::init<pylmesh::Vertex, pylmesh::Vertex, int, bool>(),
             nb::arg("min"), nb::arg("max"),
             nb::arg("bits") = 21, nb::arg("dedup") = false,
             "Create a builder with bounding box and precision")
        .def("reserve", &pylmesh::UltraQuantizedMeshBuilder::reserve,
             nb::arg("vertex_count"), nb::arg("face_count"),
             "Pre-allocate storage")
        .def("add_vertex", &pylmesh::UltraQuantizedMeshBuilder::add_vertex,
             nb::arg("x"), nb::arg("y"), nb::arg("z"),
             "Quantize and store a vertex, returns slot index")
        .def("add_face", &pylmesh::UltraQuantizedMeshBuilder::add_face,
             nb::arg("a"), nb::arg("b"), nb::arg("c"),
             "Store a triangle face from slot indices")
        .def("vertex_count", &pylmesh::UltraQuantizedMeshBuilder::vertex_count)
        .def("build",
             [](pylmesh::UltraQuantizedMeshBuilder& self) -> pylmesh::UltraQuantizedMesh
             { return std::move(self).build(); },
             "Consume the builder and return a sealed UltraQuantizedMesh");

    m.def(
        "load_ultra_quantized_mesh",
        [](const std::string& filepath)
        {
            pylmesh::UltraQuantizedMesh mesh;
            if (pylmesh::MeshLoaderFactory::load_mesh(filepath, mesh))
            {
                return mesh;
            }
            throw std::runtime_error("Failed to load ultra compressed mesh: " + filepath);
        },
        nb::arg("filepath"),
        "Load a mesh into an UltraQuantizedMesh");

    m.def(
        "save_ultra_quantized_mesh",
        [](const std::string& filepath, pylmesh::UltraQuantizedMesh& mesh)
        {
            if (pylmesh::MeshExporterFactory::save_mesh(filepath, mesh))
            {
                return true;
            }
            throw std::runtime_error("Failed to save ultra compressed mesh: " + filepath);
        },
        nb::arg("filepath"), nb::arg("mesh"),
        "Save an UltraQuantizedMesh to file");
}
