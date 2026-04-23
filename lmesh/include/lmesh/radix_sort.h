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

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <vector>

namespace pylmesh
{

// Sort `data` in-place by the `key` field of each element.
// KeyFn: uint64_t key(const T& entry)
// key_bytes: number of least-significant bytes of the key to sort on (1–8).
template <typename T, typename KeyFn>
void radix_sort(std::vector<T>& data, KeyFn key_fn, int key_bytes = 8)
{
    const size_t N = data.size();
    if (N <= 1)
        return;

    std::vector<T> buf(N);
    T* src = data.data();
    T* dst = buf.data();

    for (int byte = 0; byte < key_bytes; ++byte)
    {
        const int shift = byte * 8;

        // Count occurrences of each radix digit.
        uint32_t count[256] = {};
        for (size_t i = 0; i < N; ++i)
            ++count[(key_fn(src[i]) >> shift) & 0xFF];

        // Prefix sum → write offsets.
        uint32_t offset[256];
        offset[0] = 0;
        for (int i = 1; i < 256; ++i)
            offset[i] = offset[i - 1] + count[i - 1];

        // Scatter into dst.
        for (size_t i = 0; i < N; ++i)
        {
            uint32_t digit = (key_fn(src[i]) >> shift) & 0xFF;
            dst[offset[digit]++] = src[i];
        }

        std::swap(src, dst);
    }

    // If an odd number of passes, result is in buf — copy back.
    if (src != data.data())
        std::memcpy(data.data(), src, N * sizeof(T));
}

} // namespace pylmesh
