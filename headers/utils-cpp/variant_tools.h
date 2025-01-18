/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/utils-cpp
 * Contact:  ihor-drachuk-libs@pm.me  */

#pragma once
#include <variant>
#include <optional>
#include <utils-cpp/tuple_utils.h>

namespace utils_cpp {

namespace variant_comparisons {

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

} // namespace variant_comparisons

template<typename T, typename... Args>
std::optional<T> get_if(const std::variant<Args...>& v)
{
    if (std::holds_alternative<T>(v))
        return std::get<T>(v);

    return std::nullopt;
}

} // namespace utils_cpp
