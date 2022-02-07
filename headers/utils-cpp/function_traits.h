// Inspired by https://stackoverflow.com/a/9065203

#pragma once
#include <functional>
#include <type_traits>

template<typename T>
struct function_traits_func;

template<typename R, typename... Args>
struct function_traits_func<std::function<R(Args...)>>
{
    static constexpr size_t args_count = sizeof...(Args);

    using return_type = R;

    using args = std::tuple<Args...>;

    template <size_t I>
    struct arg
    {
        using type = typename std::tuple_element<I, args>::type;
    };
};

template<typename T>
struct function_traits : public function_traits_func< decltype(std::function(std::declval<T>())) > {};
