/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/utils-qt
 * Contact:  ihor-drachuk-libs@pm.me  */

#include <utils-cpp/xor.h>

#include <cassert>
#include <cstring>
#include <vector>

namespace utils_cpp {

namespace {

constexpr size_t SsoSize = 72;
constexpr size_t Alignment = alignof(uint64_t);

bool isAligned(const void* ptr)
{
    return (reinterpret_cast<uintptr_t>(ptr) % Alignment) == 0;
}

void xorAligned(uint64_t* dst, const uint64_t* src, size_t sz)
{
    const size_t longSz = sz / sizeof(uint64_t);
    const size_t longSzBytes = longSz * sizeof(uint64_t);

    auto* dstU64 = dst;
    const auto* srcU64 = src;

    for (size_t i = 0; i < longSz; ++i)
        dstU64[i] ^= srcU64[i];

    auto* dstTail = reinterpret_cast<uint8_t*>(dst) + longSzBytes;
    const auto* srcTail = reinterpret_cast<const uint8_t*>(src) + longSzBytes;

    for (size_t i = 0; i < sz - longSzBytes; ++i)
        dstTail[i] ^= srcTail[i];
}

} // namespace

void xorBuffer(void* dst, const void* src, size_t sz)
{
    if (sz == 0)
        return;

    assert(dst);
    assert(src);

    // Fast path: both pointers are already aligned
    if (isAligned(dst) && isAligned(src)) {
        xorAligned(reinterpret_cast<uint64_t*>(dst),
                   reinterpret_cast<const uint64_t*>(src),
                   sz);
        return;
    }

    // Slow path: copy to aligned buffers, compute, copy back
    constexpr size_t ssoU64Count = (SsoSize + sizeof(uint64_t) - 1) / sizeof(uint64_t);
    const size_t neededU64 = (sz + sizeof(uint64_t) - 1) / sizeof(uint64_t);

    uint64_t dstSso[ssoU64Count];
    uint64_t srcSso[ssoU64Count];

    uint64_t* dstAligned {};
    uint64_t* srcAligned {};
    std::vector<uint64_t> dstHeap;
    std::vector<uint64_t> srcHeap;

    if (sz <= SsoSize) {
        dstAligned = dstSso;
        srcAligned = srcSso;
    } else {
        dstHeap.resize(neededU64);
        srcHeap.resize(neededU64);
        dstAligned = dstHeap.data();
        srcAligned = srcHeap.data();
    }

    std::memcpy(dstAligned, dst, sz);
    std::memcpy(srcAligned, src, sz);

    xorAligned(dstAligned, srcAligned, sz);

    std::memcpy(dst, dstAligned, sz);
}

} // namespace utils_cpp
