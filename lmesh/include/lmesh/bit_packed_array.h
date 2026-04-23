#pragma once

// ============================================================================
//  BitPackedArray
//
//  Stores N unsigned integers each occupying exactly `width` bits (1–63).
//  Values are packed end-to-end in a uint64_t[] backing store with no padding.
//
//  Random access is O(1): each element spans at most two 64-bit words.
//  No decompression phase — just a pair of bit-shifts and a mask per access.
//
//  Memory (excluding the 8-byte guard word at the end):
//    ceil(count * width / 64) * 8  bytes
//
//  Example savings vs. fixed-width storage:
//    width=48 (3×16-bit coords)  → 6 B/elem  vs. 8 B uint64_t  = −25 %
//    width=30 (3×10-bit coords)  → 3.75 B    vs. 8 B           = −53 %
//    width=20 (index into ≤1M)   → 2.5 B     vs. 4 B uint32_t  = −37.5 %
//    width=16 (index into ≤64K)  → 2 B       vs. 4 B           = −50 %
// ============================================================================

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace pylmesh
{

class BitPackedArray
{
  public:
    BitPackedArray() = default;

    // Allocate storage for `count` elements of exactly `width` bits each.
    // width must be in [1, 63].
    BitPackedArray(uint32_t width, size_t count)
        : width_(width), count_(count)
    {
        assert(width >= 1 && width <= 63);
        const size_t total_bits = count * uint64_t(width);
        const size_t words      = (total_bits + 63) / 64;
        data_.assign(words + 1, 0); // +1: guard word so straddling reads are always safe
    }

    // Store val (masked to `width_` bits) at position i.
    void set(size_t i, uint64_t val) noexcept
    {
        const uint64_t bit_pos = static_cast<uint64_t>(i) * width_;
        const size_t   word    = static_cast<size_t>(bit_pos >> 6);
        const unsigned off     = static_cast<unsigned>(bit_pos & 63u);
        const uint64_t mask    = (1ull << width_) - 1ull;
        val &= mask;

        // Write bits that land in word `word`.
        // (mask << off) truncates naturally to 64 bits — exactly the portion
        // belonging to this word, which is what we want.
        data_[word] = (data_[word] & ~(mask << off)) | (val << off);

        // If the value straddles the boundary into word+1, write the spill.
        if (off + width_ > 64u)
        {
            const unsigned rshift = 64u - off;          // bits already written
            const unsigned sbits  = (off + width_) - 64u; // bits spilling over
            const uint64_t smask  = (1ull << sbits) - 1ull;
            data_[word + 1] = (data_[word + 1] & ~smask) | (val >> rshift);
        }
    }

    // Retrieve the value at position i (zero-extended to uint64_t).
    [[nodiscard]] uint64_t get(size_t i) const noexcept
    {
        const uint64_t bit_pos = static_cast<uint64_t>(i) * width_;
        const size_t   word    = static_cast<size_t>(bit_pos >> 6);
        const unsigned off     = static_cast<unsigned>(bit_pos & 63u);
        const uint64_t mask    = (1ull << width_) - 1ull;

        uint64_t val = data_[word] >> off;
        if (off + width_ > 64u)
            val |= data_[word + 1] << (64u - off);
        return val & mask;
    }

    size_t   count()      const noexcept { return count_; }
    uint32_t width()      const noexcept { return width_; }

    // Bytes actually used for data (excludes guard word).
    size_t bytes_used() const noexcept
    {
        return data_.empty() ? 0 : (data_.size() - 1) * sizeof(uint64_t);
    }

    void shrink_to_fit() { data_.shrink_to_fit(); }

  private:
    std::vector<uint64_t> data_;
    uint32_t width_ = 0;
    size_t   count_ = 0;
};

} // namespace pylmesh