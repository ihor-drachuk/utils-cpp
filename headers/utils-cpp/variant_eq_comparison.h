/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/utils-cpp
 * Contact:  ihor-drachuk-libs@pm.me  */

#pragma once
#include <variant>
#include <utils-cpp/tuple_utils.h>

template<typename T, typename... Args>
bool operator==(const std::variant<Args...>& lhs, const T& rhs)
{
    if constexpr (contains_type<T, Args...>::value) {
        if (!std::holds_alternative<T>(lhs))
            return false;

        return (std::get<T>(lhs) == rhs);
    } else {
        return false;
    }
}

template<typename T, typename... Args>
bool operator==(const T& lhs, const std::variant<Args...>& rhs) { return (rhs == lhs); }

template<typename T, typename... Args>
bool operator!=(const std::variant<Args...>& lhs, const T& rhs) { return !(lhs == rhs); }

template<typename T, typename... Args>
bool operator!=(const T& lhs, const std::variant<Args...>& rhs) { return !(lhs == rhs); }
