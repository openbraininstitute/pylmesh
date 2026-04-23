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

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <vector>

namespace pylmesh
{

/**
 * FlatHashMap<uint64_t, uint32_t>
 *
 * Open-addressing hash map with linear probing, purpose-built for the
 * QuantizedMeshBuilder dedup path.
 *
 * Memory layout: one contiguous array of {uint64_t key, uint32_t value}
 * slots — no heap nodes, no pointer chasing, no allocator overhead.
 *
 *    unordered_map  : ~48 bytes/entry  (node + bucket pointer + allocator)
 *    FlatHashMap    : ~12 bytes/slot   (key + value, no indirection)
 *
 * Effective bytes/entry at the actual load factors below (power-of-2 capacity):
 *    N=100K  → load 38% → ~31 B/entry
 *    N=1M    → load 48% → ~25 B/entry
 *    N=10M   → load 60% → ~20 B/entry
 *
 * Sentinel: EMPTY_KEY = UINT64_MAX. Safe because valid encoded vertices
 *  use at most 3×21 = 63 bits, so bit 63 is always 0 for real keys.
 *
 * No deletion support (not needed in the build-then-discard lifecycle).
 */
class FlatHashMap
{
  public:
    static constexpr uint64_t EMPTY_KEY = std::numeric_limits<uint64_t>::max();

    FlatHashMap() = default;

    // Reserve capacity for at least `n` entries while staying at ≤75% load.
    void reserve(size_t n)
    {
        const size_t cap = next_pow2(n * 4 / 3 + 1);
        if (cap > slots_.size())
            grow(cap);
    }

    // If `key` is already present, return {existing_value, false}.
    // If `key` is absent,  insert (key, value) and return {value, true}.
    std::pair<uint32_t, bool> emplace(uint64_t key, uint32_t value)
    {
        assert(key != EMPTY_KEY && "key must not equal the sentinel");

        // Grow before inserting so we never search a full table.
        if ((size_ + 1) * 4 > slots_.size() * 3) [[unlikely]]
            grow(std::max(slots_.size() * 2, size_t(16)));

        size_t idx = mix(key) & mask_;
        while (true)
        {
            Slot& s = slots_[idx];
            if (s.key == EMPTY_KEY)
            {
                s.key = key;
                s.value = value;
                ++size_;
                return {value, true};
            }
            if (s.key == key)
                return {s.value, false};

            idx = (idx + 1) & mask_; // linear probe — stays cache-hot
        }
    }

    // Release all memory. Called at the start of QuantizedMeshBuilder::build().
    void clear_and_free()
    {
        std::vector<Slot>{}.swap(slots_);
        size_ = 0;
        mask_ = 0;
    }

    size_t size() const noexcept
    {
        return size_;
    }
    size_t capacity() const noexcept
    {
        return slots_.size();
    }
    size_t bytes_used() const noexcept
    {
        return slots_.size() * sizeof(Slot);
    }

  private:
    struct Slot
    {
        uint64_t key = EMPTY_KEY;
        uint32_t value = 0;
    };

    std::vector<Slot> slots_;
    size_t size_ = 0;
    size_t mask_ = 0; // capacity − 1; valid only when slots_ is non-empty

    // Rehash into a fresh table of exactly `new_cap` slots (must be power of 2).
    void grow(size_t new_cap)
    {
        assert((new_cap & (new_cap - 1)) == 0 && "capacity must be a power of 2");
        std::vector<Slot> fresh(new_cap); // zero-initialised → key = EMPTY_KEY
        const size_t new_mask = new_cap - 1;

        for (const Slot& s : slots_)
        {
            if (s.key == EMPTY_KEY)
                continue;
            size_t idx = mix(s.key) & new_mask;
            while (fresh[idx].key != EMPTY_KEY)
                idx = (idx + 1) & new_mask;
            fresh[idx] = s;
        }

        slots_ = std::move(fresh);
        mask_ = new_mask;
    }

    // Bit-mixing finaliser (Murmur3-style) — distributes the packed grid
    // coordinates evenly across the bucket array.
    static uint64_t mix(uint64_t k) noexcept
    {
        k ^= k >> 33;
        k *= 0xff51afd7ed558ccdULL;
        k ^= k >> 33;
        k *= 0xc4ceb9fe1a85ec53ULL;
        k ^= k >> 33;
        return k;
    }

    static size_t next_pow2(size_t n) noexcept
    {
        if (n <= 1)
            return 1;
        --n;
        n |= n >> 1;
        n |= n >> 2;
        n |= n >> 4;
        n |= n >> 8;
        n |= n >> 16;
        n |= n >> 32;
        return n + 1;
    }
};

} // namespace pylmesh