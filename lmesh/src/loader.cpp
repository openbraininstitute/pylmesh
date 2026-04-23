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

#include "lmesh/loader.h"
#include "lmesh/loaders/gltf_loader.h"
#include "lmesh/loaders/obj_loader.h"
#include "lmesh/loaders/off_loader.h"
#include "lmesh/loaders/ply_loader.h"
#include "lmesh/loaders/stl_loader.h"

namespace pylmesh
{

std::unique_ptr<MeshLoader> MeshLoaderFactory::createLoader(const std::string& filepath)
{
    if (OBJLoader().canLoad(filepath))
    {
        return std::make_unique<OBJLoader>();
    }
    if (STLLoader().canLoad(filepath))
    {
        return std::make_unique<STLLoader>();
    }
    if (PLYLoader().canLoad(filepath))
    {
        return std::make_unique<PLYLoader>();
    }
    if (OFFLoader().canLoad(filepath))
    {
        return std::make_unique<OFFLoader>();
    }
    if (GLTFLoader().canLoad(filepath))
    {
        return std::make_unique<GLTFLoader>();
    }
    return nullptr;
}

bool MeshLoaderFactory::loadMesh(const std::string& filepath, Mesh& mesh)
{
    auto loader = createLoader(filepath);
    if (!loader)
        return false;
    return loader->load(filepath, mesh);
}

bool MeshLoaderFactory::loadMesh(const std::string& filepath, QuantizedMesh& mesh)
{
    auto loader = createLoader(filepath);
    if (!loader)
        return false;
    return loader->load(filepath, mesh);
}

bool MeshLoaderFactory::loadMesh(const std::string& filepath, UltraQuantizedMesh& mesh)
{
    auto loader = createLoader(filepath);
    if (!loader)
        return false;
    return loader->load(filepath, mesh);
}

} // namespace pylmesh
