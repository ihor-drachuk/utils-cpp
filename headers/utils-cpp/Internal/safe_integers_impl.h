/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/utils-qt
 * Contact:  ihor-drachuk-libs@pm.me  */

#pragma once
#include <cassert>
#include <cstdint>
#include <limits>
#include <type_traits>

namespace utils_cpp_internal {

template<typename T, bool IsEnum> struct UnwrapEnumImpl {};
template<typename T>              struct UnwrapEnumImpl<T, false> { using Type = T; };
template<typename T>              struct UnwrapEnumImpl<T, true>  { using Type = std::underlying_type_t<T>; };

template<typename T>              struct UnwrapEnum : UnwrapEnumImpl<T, std::is_enum_v<T>> {};

template<typename T>
constexpr void checkIntegralType()
{
    static_assert(std::is_integral_v<T> || std::is_enum_v<T>, "Expected integral type!");
    static_assert(!std::is_same_v<T, bool>, "Boolean type is not expected!");
}

template<size_t N> struct Signed {};
template<>         struct Signed<1> { using Type = int8_t;  using NextType = int16_t; };
template<>         struct Signed<2> { using Type = int16_t; using NextType = int32_t; };
template<>         struct Signed<4> { using Type = int32_t; using NextType = int64_t; };
template<>         struct Signed<8> { using Type = int64_t; };

template<size_t N> struct Unsigned {};
template<>         struct Unsigned<1> { using Type = uint8_t;  };
template<>         struct Unsigned<2> { using Type = uint16_t; };
template<>         struct Unsigned<4> { using Type = uint32_t; };
template<>         struct Unsigned<8> { using Type = uint64_t; };

template<bool ToSigned, bool Safe = false, typename T>
auto castSignness(T value)
{
    checkIntegralType<T>();
    using U = typename UnwrapEnum<T>::Type;

    if constexpr (ToSigned == std::is_signed_v<U>) {
        return static_cast<U>(value);

    } else {
        if constexpr (ToSigned) {
            if constexpr (Safe) {
                using R = typename Signed<sizeof(T)>::NextType;
                return static_cast<R>(value);

            } else {
                using R = typename Signed<sizeof(T)>::Type;

                assert(static_cast<U>(value) <= std::numeric_limits<R>::max());
                return static_cast<R>(value);
            }

        } else {
            using R = typename Unsigned<sizeof(T)>::Type;

            assert(static_cast<U>(value) >= 0);
            return static_cast<R>(value);
        }
    }
}

template<typename R, typename T>
R arbitraryCast(T value)
{
    checkIntegralType<T>();
    checkIntegralType<R>();
    assert(value >= std::numeric_limits<R>::min());
    assert(value <= std::numeric_limits<R>::max());
    return static_cast<R>(value);
}

template<typename A, typename B>
int compareIntegers(A lhs, B rhs)
{
    checkIntegralType<A>();
    checkIntegralType<B>();

    // codechecker_intentional [clang-diagnostic-sign-compare]
    if (std::numeric_limits<A>::max() < rhs)
        return -1;

    // codechecker_intentional [clang-diagnostic-sign-compare]
    if (std::numeric_limits<B>::max() < lhs)
        return 1;

    // codechecker_intentional [clang-diagnostic-sign-compare]
    return lhs < rhs ? -1 : lhs > rhs ? 1 : 0;
}

template<typename T>
class SafeIntegerComparer
{
    template<typename U> friend class SafeIntegerComparer;
public:
    SafeIntegerComparer(T value): m_value(value) {}

    template<typename U> bool operator==(U other) const { return compareIntegers(m_value, other) == 0; }
    template<typename U> bool operator!=(U other) const { return compareIntegers(m_value, other) != 0; }
    template<typename U> bool operator<(U other) const { return compareIntegers(m_value, other) < 0; }
    template<typename U> bool operator>(U other) const { return compareIntegers(m_value, other) > 0; }
    template<typename U> bool operator<=(U other) const { return compareIntegers(m_value, other) <= 0; }
    template<typename U> bool operator>=(U other) const { return compareIntegers(m_value, other) >= 0; }

private:
    T m_value;
};

template<typename T>
SafeIntegerComparer(T) -> SafeIntegerComparer<T>;

} // namespace utils_cpp_internal
