// Inspired by https://stackoverflow.com/a/9065203

#pragma once
#include <functional>
#include <type_traits>

template<typename T>
struct function_traits;

template<typename R, typename... Args>
struct function_traits<std::function<R(Args...)>>
{
    static constexpr size_t args_count = sizeof...(Args);

    using result_type = R;

    template <size_t I>
    struct arg
    {
        using type = typename std::tuple_element<I, std::tuple<Args...>>::type;
    };
};
