#pragma once
#include <tuple>
#include <type_traits>
#include <utils-cpp/function_traits.h>

//
// tuple_cat_type<tupleA, tupleB>  ->  tupleAB
//

template<typename, typename>
struct tuple_cat_type;

template<typename... Args1, typename... Args2>
struct tuple_cat_type<std::tuple<Args1...>, std::tuple<Args2...>>
{
    using type = std::tuple<Args1..., Args2...>;
};


//
// duplicate_type<3, int>::type  ->  std::tuple<int, int, int>
//

template<size_t N, typename T>
struct duplicate_type_impl
{
    using type = typename tuple_cat_type<std::tuple<T>, typename duplicate_type_impl<N-1, T>::type>::type;
};

template<typename T>
struct duplicate_type_impl<1, T>
{
    using type = std::tuple<T>;
};

template<typename T>
struct duplicate_type_impl<0, T> { };

template<typename T, size_t N>
struct duplicate_type : public duplicate_type_impl<N, T> { };


//
// is_tuple<T>::value
// is_tuple_v<T>
//

template<typename T>
struct is_tuple : public std::false_type {};

template<typename... Ts>
struct is_tuple<std::tuple<Ts...>> : public std::true_type {};

template<typename T>
constexpr bool is_tuple_v = is_tuple<T>::value;


//
// std::integer_sequence  ->  std::tuple
//

template<typename T, T... Is>
auto integer_sequence_to_tuple(const std::integer_sequence<T, Is...>&)
{
    return std::make_tuple(Is...);
}


//
// call_multiple(func, arg0, args1, ...)
// call_multiple_tuple(func, std::tuple<arg0, arg1, ...>)
//

template<typename T>
struct tuplize
{
    using type = std::tuple<T>;

    tuplize(const T& value): value(std::tie(value)) {}
    std::tuple<const T&> value;
};

template<typename... Args>
struct tuplize<std::tuple<Args...>>
{
    using type = std::tuple<Args...>;

    tuplize(const type& value): value(value) {}
    const type& value;
};

template<size_t I, typename Ret, typename Func,
         typename std::enable_if<I >= std::tuple_size<Ret>::value>::type* = nullptr>
void call_multiple_I(Ret&, const Func&)
{
}

template<size_t I, typename Ret, typename Func, typename Arg0, typename... Args,
         typename std::enable_if<!(I >= std::tuple_size<Ret>::value)>::type* = nullptr>
void call_multiple_I(Ret& ret, const Func& func, const Arg0& arg0, const Args&... args)
{
    std::get<I>(ret) = std::apply(func, tuplize<Arg0>(arg0).value);
    call_multiple_I<I+1, Ret>(ret, func, args...);
}

template<typename Func, typename Arg0, typename... Args, typename Ret = typename duplicate_type<decltype(std::apply(std::declval<Func>(), std::declval<typename tuplize<Arg0>::type>())), sizeof...(Args)+1>::type>
Ret call_multiple(const Func& func, const Arg0& arg0, const Args&... args)
{
    Ret ret;

    call_multiple_I<0, Ret>(ret, func, arg0, args...);

    return ret;
}

template<typename Func, typename Arg0, typename... Args>
auto call_multiple_tuple(const Func& func, const std::tuple<Arg0, Args...>& allArgs)
{
    auto params = std::tuple_cat(std::tie(func),
                                 allArgs);

    return std::apply(call_multiple<Func, Arg0, Args...>, params);
}
