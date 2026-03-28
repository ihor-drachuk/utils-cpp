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

    using UA = typename UnwrapEnum<A>::Type;
    using UB = typename UnwrapEnum<B>::Type;

    constexpr bool a_signed = std::is_signed_v<UA>;
    constexpr bool b_signed = std::is_signed_v<UB>;

    if constexpr (a_signed == b_signed) {
        // Same signedness: implicit promotion to the larger type is safe
        return lhs < rhs ? -1 : lhs > rhs ? 1 : 0;
    } else if constexpr (a_signed) {
        // lhs is signed, rhs is unsigned: negative signed is always less
        if (lhs < 0) return -1;
        // lhs >= 0, compare in common unsigned type
        using Common = std::conditional_t<(sizeof(UA) > sizeof(UB)),
                                          std::make_unsigned_t<UA>, UB>;
        return static_cast<Common>(lhs) < static_cast<Common>(rhs) ? -1
             : static_cast<Common>(lhs) > static_cast<Common>(rhs) ? 1 : 0;
    } else {
        // lhs is unsigned, rhs is signed: negative signed is always less
        if (rhs < 0) return 1;
        // rhs >= 0, compare in common unsigned type
        using Common = std::conditional_t<(sizeof(UB) > sizeof(UA)),
                                          std::make_unsigned_t<UB>, UA>;
        return static_cast<Common>(lhs) < static_cast<Common>(rhs) ? -1
             : static_cast<Common>(lhs) > static_cast<Common>(rhs) ? 1 : 0;
    }
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
