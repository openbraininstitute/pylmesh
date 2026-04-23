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
#include "lmesh/exporters/gltf_exporter.h"
#include "lmesh/exporters/obj_exporter.h"
#include "lmesh/exporters/off_exporter.h"
#include "lmesh/exporters/ply_exporter.h"
#include "lmesh/exporters/stl_exporter.h"

namespace pylmesh
{

std::unique_ptr<MeshExporter> MeshExporterFactory::create_exporter(const std::string& filepath)
{
    if (OBJExporter().can_save(filepath))
    {
        return std::make_unique<OBJExporter>();
    }
    if (STLExporter().can_save(filepath))
    {
        return std::make_unique<STLExporter>();
    }
    if (PLYExporter().can_save(filepath))
    {
        return std::make_unique<PLYExporter>();
    }
    if (OFFExporter().can_save(filepath))
    {
        return std::make_unique<OFFExporter>();
    }
    if (GLTFExporter().can_save(filepath))
    {
        return std::make_unique<GLTFExporter>();
    }
    if (GLBExporter().can_save(filepath))
    {
        return std::make_unique<GLBExporter>();
    }
    return nullptr;
}

bool MeshExporterFactory::save_mesh(const std::string& filepath, const Mesh& mesh)
{
    auto exporter = create_exporter(filepath);
    if (!exporter)
        return false;
    return exporter->save(filepath, mesh);
}

bool MeshExporterFactory::save_mesh(const std::string& filepath, const QuantizedMesh& mesh)
{
    auto exporter = create_exporter(filepath);
    if (!exporter)
        return false;
    return exporter->save(filepath, mesh);
}

bool MeshExporterFactory::save_mesh(const std::string& filepath, UltraQuantizedMesh& mesh)
{
    auto exporter = create_exporter(filepath);
    if (!exporter)
        return false;
    return exporter->save(filepath, mesh);
}

} // namespace pylmesh
