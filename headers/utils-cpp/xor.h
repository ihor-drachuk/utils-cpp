/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/utils-qt
 * Contact:  ihor-drachuk-libs@pm.me  */

#pragma once
#include <type_traits>
#include <cstdint>
#include <cstddef>

namespace utils_cpp {

namespace xor_detail {
template<typename T>
constexpr bool is_byte_type_v = std::is_same_v<T, char>
                             || std::is_same_v<T, unsigned char>
                             || std::is_same_v<T, signed char>
                             || std::is_same_v<T, uint8_t>
                             || std::is_same_v<T, int8_t>
                             || std::is_same_v<T, std::byte>;
} // namespace xor_detail

template<typename A, typename B,
         typename = std::enable_if_t<xor_detail::is_byte_type_v<A> &&
                                     xor_detail::is_byte_type_v<B>>>
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
