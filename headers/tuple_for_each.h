#pragma once
#include <tuple>
#include <utility>
#include <optional>

template<std::size_t I = 0, typename FuncT, typename... Tp>
inline typename std::enable_if<I == sizeof...(Tp), void>::type
for_each(const std::tuple<Tp...> &, FuncT) // Unused arguments are given no names.
{
}

template<std::size_t I = 0, typename FuncT, typename... Tp>
inline typename std::enable_if<I < sizeof...(Tp), void>::type
for_each(const std::tuple<Tp...>& t, FuncT f)
{
    f(I, std::get<I>(t));
    for_each<I + 1, FuncT, Tp...>(t, f);
}

template<typename Param, std::size_t I = 0, typename FuncT, typename... Tp>
inline typename std::enable_if<I == sizeof...(Tp), std::optional<Param>>::type
for_each_par(const std::tuple<Tp...> &, FuncT, const std::optional<Param>& param, bool = false) // Unused arguments are given no names.
{
    return param;
}

template<typename Param, std::size_t I = 0, typename FuncT, typename... Tp>
inline typename std::enable_if<I < sizeof...(Tp), std::optional<Param>>::type
for_each_par(const std::tuple<Tp...>& t, FuncT f, const std::optional<Param>& param, bool exitOnResult = false)
{
    auto val = f(I, std::get<I>(t), param);
    if (exitOnResult && val) return val;
    return for_each_par<Param, I + 1, FuncT, Tp...>(t, f, val, exitOnResult);
}

template<typename Param, std::size_t I = 0, typename FuncT, typename... Tp>
inline typename std::enable_if<I == sizeof...(Tp), std::optional<Param>>::type
for_each_pair_par(const std::tuple<Tp...> &, const std::tuple<Tp...> &, FuncT, const std::optional<Param>& param, bool = false) // Unused arguments are given no names.
{
    return param;
}

template<typename Param, std::size_t I = 0, typename FuncT, typename... Tp>
inline typename std::enable_if<I < sizeof...(Tp), std::optional<Param>>::type
for_each_pair_par(const std::tuple<Tp...>& t, const std::tuple<Tp...>& t2, FuncT f, const std::optional<Param>& param, bool exitOnResult = false)
{
    const auto val = f(I, std::get<I>(t), std::get<I>(t2), param);
    if (exitOnResult && val) return val;
    return for_each_pair_par<Param, I + 1, FuncT, Tp...>(t, t2, f, val, exitOnResult);
}
