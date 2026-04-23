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

#include <array>
#include <cstdint>

#include "vertex.h"

namespace pylmesh
{

class BaseMesh
{
  public:
    using Face = std::array<uint32_t, 3>;

    virtual ~BaseMesh() = default;

    virtual Vertex get_vertex(uint32_t i) const = 0;
    virtual Face get_face(uint32_t i) const = 0;
    virtual uint32_t vertex_count() const noexcept = 0;
    virtual uint32_t face_count() const noexcept = 0;
    virtual double surface_area() const = 0;
    virtual size_t vertex_bytes() const noexcept = 0;
    virtual size_t face_bytes() const noexcept = 0;
    virtual size_t total_bytes() const noexcept = 0;
};

} // namespace pylmesh
