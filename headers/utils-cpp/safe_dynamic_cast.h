/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/utils-cpp
 * Contact:  ihor-drachuk-libs@pm.me  */

#pragma once
#include <cassert>
#include <type_traits>

template<typename T, typename S>
T safe_dynamic_cast(S ptr)
{
    static_assert ((std::is_pointer_v<S> && std::is_pointer_v<T>) ||
                   (std::is_reference_v<S> && std::is_reference_v<T>), "Both S and T must be either pointers or references!");

    auto result = dynamic_cast<T>(ptr);

    if constexpr (std::is_pointer_v<T>)
        assert(result);

    return result;
}
