/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/utils-cpp
 * Contact:  ihor-drachuk-libs@pm.me  */

#pragma once
#include <tuple>
#include <type_traits>
#include <utils-cpp/function_traits.h>

//
// Compile-time int
//
template<size_t I_>
struct Int
{
    constexpr static size_t I = I_;
};

//
// contains_type
//

template<typename... Args>
struct contains_type : std::false_type {};

template<typename T, typename Arg0, typename... Args>
struct contains_type<T, Arg0, Args...> : std::conditional_t<std::is_same_v<T, Arg0>, std::true_type, contains_type<T, Args...>>
{};

template<typename T, typename Tuple>
struct contains_type_tuple : std::false_type {};

template<typename T, typename Arg0, typename... Args>
struct contains_type_tuple<T, std::tuple<Arg0, Args...>> : std::conditional_t<std::is_same_v<T, Arg0>, std::true_type, contains_type_tuple<T, std::tuple<Args...>>>
{};

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
struct duplicate_type_impl<0, T>
{
    using type = std::tuple<>;
};

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
// for_each_tuple_pair(func, tuple1, tuple2)
// I.e.:
//   - func(std::get<0>(tuple1), std::get<0>(tuple2), Int<0>);
//   - func(std::get<1>(tuple1), std::get<1>(tuple2), Int<1>);
//   - ...
//

template <typename F, typename Tuple1, typename Tuple2, std::size_t... I>
void for_each_tuple_pair_impl(F&& f, Tuple1&& tuple1, Tuple2&& tuple2, std::index_sequence<I...>)
{
    (f(std::get<I>(tuple1), std::get<I>(tuple2), Int<I>()), ...);
}

template <typename F, typename Tuple1, typename Tuple2>
void for_each_tuple_pair(F&& f, Tuple1&& tuple1, Tuple2&& tuple2)
{
    constexpr size_t Len = std::tuple_size_v<std::remove_cv_t<std::remove_reference_t<Tuple1>>>;
    for_each_tuple_pair_impl(f, tuple1, tuple2, std::make_index_sequence<Len>());
}


//
// for_each(func, arg0, args1, ...)
// for_each_tuple(func, std::tuple<arg0, arg1, ...>)
// I.e.:
//   - func(arg0);  func(arg1);  ...
//   - std::make_tuple(func(arg0), func(arg1), ...)
//

namespace utils_cpp_internal {

template<typename Selector>
struct universal_call_impl
{
    template<typename Func, typename Param>
    static auto call(Func&& func, Param&& param)
    {
        return func(param);
    }
};

template<typename... Ts>
struct universal_call_impl<std::tuple<Ts...>>
{
    template<typename Func, typename Param>
    static auto call(Func&& func, Param&& tuple)
    {
        return std::apply(func, tuple);
    }
};

template<typename Func, typename T>
auto universal_call(Func&& func, T&& param)
{
    using Selector = std::remove_cv_t<std::remove_reference_t<T>>;
    return universal_call_impl<Selector>::call(func, param);
}

struct for_each_impl_void
{
    template<typename F, typename Tuple>
    static void for_each_tuple(F&& func, Tuple&& params)
    {
        std::apply([&func](auto&&... xs){ (universal_call(func, xs), ...); }, params);
    }
};

struct for_each_impl
{
    template<typename F, typename Tuple,
             typename TupleRaw = std::remove_cv_t<std::remove_reference_t<Tuple>>,
             typename FuncRet = typename function_traits<F>::return_type,
             typename OwnRet = typename duplicate_type<FuncRet, std::tuple_size_v<TupleRaw>>::type>
    static auto for_each_tuple(F&& func, Tuple&& params)
    {
        OwnRet results;

        for_each_tuple_pair([&func](auto&& result, auto&& param, auto){
            result = universal_call(func, param);
        }, results, params);

        return results;
    }
};

template<typename Func,
         typename RetType = typename function_traits<Func>::return_type>
struct for_each_impl_selector : std::conditional_t<
                                    std::is_same_v<RetType, void>,
                                    for_each_impl_void,
                                    for_each_impl
                                >
{ };

} // namespace utils_cpp_internal

template<typename F, typename... Ts>
auto for_each_tuple(F&& func, std::tuple<Ts...>&& params)
{
    return utils_cpp_internal::for_each_impl_selector<F>::for_each_tuple(func, params);
}

template<typename F, typename... Ts>
auto for_each(F&& func, Ts&&... params)
{
    return for_each_tuple(func, std::tie(params...));
}
