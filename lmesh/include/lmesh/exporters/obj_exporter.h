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
#include "../exporter.h"

namespace pylmesh
{

class OBJExporter : public MeshExporter
{
  public:
    bool save(const std::string& filepath, const Mesh& mesh) override;
    bool save(const std::string& filepath, const QuantizedMesh& mesh) override;
    bool save(const std::string& filepath, UltraQuantizedMesh& mesh) override;
    bool can_save(const std::string& filepath) const override;
};

} // namespace pylmesh
