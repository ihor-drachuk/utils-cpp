#pragma once
#include <type_traits>
#include <iterator>

template<bool... b> constexpr bool variadic_and = true && (b && ...);
template<bool... b> constexpr bool variadic_or = (b || ...);

template<template<typename...> class Container,
         typename DestType,
         typename... Ts,
         typename std::enable_if<variadic_and<std::is_convertible<Ts, DestType>::value...>>* = nullptr>
Container<DestType> variadic_to_container(const Ts&... values)
{
    Container<DestType> vec;
    auto appendIt = std::back_inserter(vec);
    auto appender = [&appendIt](const DestType& x){ appendIt++ = x; };
    (appender(values), ...);
    return vec;
}
