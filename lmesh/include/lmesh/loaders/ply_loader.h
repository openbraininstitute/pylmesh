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

#pragma once
#include "../loader.h"

namespace pylmesh
{

class PLYLoader : public MeshLoader
{
  public:
    bool load(const std::string& filepath, Mesh& mesh) override;
    bool load(const std::string& filepath, QuantizedMesh& mesh) override;
    bool canLoad(const std::string& filepath) const override;
};

} // namespace pylmesh
