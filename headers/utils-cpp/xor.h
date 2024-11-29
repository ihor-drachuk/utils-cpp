/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/utils-qt
 * Contact:  ihor-drachuk-libs@pm.me  */

#pragma once
#include <type_traits>
#include <cstdint>
#include <cstddef>

namespace utils_cpp {

template<typename A, typename B,
         typename = std::enable_if_t<sizeof(A) == 1 && sizeof(B) == 1>>
void xorByte(A& x, B y)
{
    reinterpret_cast<uint8_t&>(x) ^= reinterpret_cast<uint8_t&>(y);
}

void xorBuffer(void* dst, const void* src, size_t sz);

template<typename It1, typename It2>
void xorBufferIt(It1 itDst, It1 itDstEnd, It2 itSrc)
{
    while (itDst != itDstEnd)
        *itDst++ ^= *itSrc++;
}

} // namespace utils_cpp
