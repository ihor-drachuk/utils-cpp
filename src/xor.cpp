/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/utils-cpp
 * Contact:  ihor-drachuk-libs@pm.me  */

#include <utils-cpp/xor.h>

namespace utils_cpp {

void xorBuffer(void* dst, const void* src, size_t sz)
{
    const size_t longSz = sz / sizeof(uint64_t);
    const size_t longSzBytes = longSz * sizeof(uint64_t);

    auto* dstPtrU64 = reinterpret_cast<uint64_t*>(dst);
    const auto* dstPtrEndU64 = reinterpret_cast<uint64_t*>(dst) + longSz;
    const auto* srcPtrU64 = reinterpret_cast<const uint64_t*>(src);

    while (dstPtrU64 != dstPtrEndU64)
       *dstPtrU64++ ^= *srcPtrU64++;

    auto* dstPtr = reinterpret_cast<uint8_t*>(dst) + longSzBytes;
    const auto* dstPtrEnd = reinterpret_cast<uint8_t*>(dst) + sz;
    const auto* srcPtr = reinterpret_cast<const uint8_t*>(src) + longSzBytes;

    while (dstPtr != dstPtrEnd)
       *dstPtr++ ^= *srcPtr++;
}

} // namespace utils_cpp
