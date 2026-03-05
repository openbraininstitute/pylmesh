#include "pylmesh/exporter.h"
#include "pylmesh/exporters/obj_exporter.h"
#include "pylmesh/exporters/stl_exporter.h"
#include "pylmesh/exporters/ply_exporter.h"
#include "pylmesh/exporters/off_exporter.h"
#include "pylmesh/exporters/gltf_exporter.h"

namespace pylmesh {

std::unique_ptr<MeshExporter> MeshExporterFactory::createExporter(const std::string& filepath) {
    if (OBJExporter().canSave(filepath)) {
        return std::make_unique<OBJExporter>();
    }
    if (STLExporter().canSave(filepath)) {
        return std::make_unique<STLExporter>();
    }
    if (PLYExporter().canSave(filepath)) {
        return std::make_unique<PLYExporter>();
    }
    if (OFFExporter().canSave(filepath)) {
        return std::make_unique<OFFExporter>();
    }
    if (GLTFExporter().canSave(filepath)) {
        return std::make_unique<GLTFExporter>();
    }
    if (GLBExporter().canSave(filepath)) {
        return std::make_unique<GLBExporter>();
    }
    return nullptr;
}

bool MeshExporterFactory::saveMesh(const std::string& filepath, const Mesh& mesh) {
    auto exporter = createExporter(filepath);
    if (!exporter) return false;
    return exporter->save(filepath, mesh);
}

}
