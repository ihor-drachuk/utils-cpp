/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/utils-cpp
 * Contact:  ihor-drachuk-libs@pm.me  */

#pragma once
#include <utility>

template<typename T>
T shift(T& a, T&& b)
{
    T temp = std::move(a);
    a = std::move(b);
    return temp;
}
