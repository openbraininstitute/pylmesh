#pragma once

// ============================================================================
//  radix_sort.h — LSB radix sort for fixed-width key + payload structs
//
//  8-bit radix (256 buckets), one pass per byte of the key.
//  O(N × key_bytes) time, O(N) extra space (ping-pong buffer).
//
//  For 11.7M entries:
//    std::sort  : ~25 s  (comparison-based, cache-unfriendly)
//    radix_sort : ~1–2 s (linear, sequential access)
// ============================================================================

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
    if (N <= 1) return;

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
