/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/utils-qt
 * Contact:  ihor-drachuk-libs@pm.me  */

#pragma once
#include <utils-cpp/Internal/safe_integers_impl.h>

/*
                Description
-------------------------------------------
There are functions to perform safe casting between signed and unsigned integers,
to perform arbitrary casting and to compare integers reliably.

  *  u(sint)  -> unsigned integer
  *  s(uint)  -> signed integer
  *  ss(uint) -> signed integer of extended size (guarantees no data loss)

  *  underlying(enum) -> underlying integer type

  *  u8 (sint) -> uint8_t
  *  u16(sint) -> uint16_t
  *  u32(sint) -> uint32_t
  *  u64(sint) -> uint64_t

  *  s8 (uint) -> int8_t
  *  s16(uint) -> int16_t
  *  s32(uint) -> int32_t
  *  s64(uint) -> int64_t

  *  c(int)    -> SafeIntegerComparer
     Guarantees true comparison of signed and unsigned integers of different sizes.
     Usage example:
       Before                        After
         myCounter == 34               c(myCounter) == 34
         myCounter > Max               c(myCounter) > Max
         myCounter < size()            c(myCounter) < size()
*/

template<typename T> auto u(T value) { return utils_cpp_internal::castSignness<false>(value); }
template<typename T> auto s(T value) { return utils_cpp_internal::castSignness<true>(value); }
template<typename T> auto ss(T value) { return utils_cpp_internal::castSignness<true, true>(value); }

template<typename T> auto underlying(T value)
{
    static_assert(std::is_enum<T>::value, "Argument should be an enum!");
    return static_cast<typename utils_cpp_internal::UnwrapEnum<T>::Type>(value);
}

template<typename T> auto u8(T value) { return utils_cpp_internal::arbitraryCast<uint8_t>(value); }
template<typename T> auto u16(T value) { return utils_cpp_internal::arbitraryCast<uint16_t>(value); }
template<typename T> auto u32(T value) { return utils_cpp_internal::arbitraryCast<uint32_t>(value); }
template<typename T> auto u64(T value) { return utils_cpp_internal::arbitraryCast<uint64_t>(value); }
template<typename T> auto s8(T value) { return utils_cpp_internal::arbitraryCast<int8_t>(value); }
template<typename T> auto s16(T value) { return utils_cpp_internal::arbitraryCast<int16_t>(value); }
template<typename T> auto s32(T value) { return utils_cpp_internal::arbitraryCast<int32_t>(value); }
template<typename T> auto s64(T value) { return utils_cpp_internal::arbitraryCast<int64_t>(value); }
template<typename T> auto toSizeT(T value) { return utils_cpp_internal::arbitraryCast<size_t>(value); }

template<typename T> auto c(T value) { return utils_cpp_internal::SafeIntegerComparer(value); }
