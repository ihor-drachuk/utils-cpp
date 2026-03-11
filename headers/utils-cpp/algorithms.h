/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/utils-cpp
 * Contact:  ihor-drachuk-libs@pm.me  */

#pragma once
#include <algorithm>
#include <chrono>
#include <numeric>
#include <type_traits>
#include <utility>

namespace utils_cpp {

namespace Internals {

template<typename T>
struct is_duration : std::false_type {};

template<typename Rep, typename Period>
struct is_duration<std::chrono::duration<Rep, Period>> : std::true_type {};

template<typename T0, typename... Ts>
struct Arg0
{
    using type = T0;
};

template<typename... Ts>
using Arg0_t = typename Arg0<Ts...>::type;

template<typename... Ts> using SelectCommonType = std::common_type_t<Ts...>;
// template<typename... Ts> using SelectCommonType = Arg0_t<Ts...>;

} // namespace Internals

template<typename... Args>
constexpr auto min(Args&&... args)
{
    using Ret = Internals::SelectCommonType<Args...>;
    return std::min({static_cast<Ret>(std::forward<Args>(args))...});
}

template<typename... Args>
constexpr auto max(Args&&... args)
{
    using Ret = Internals::SelectCommonType<Args...>;
    return std::max({static_cast<Ret>(std::forward<Args>(args))...});
}

// Greatest common divisor
template <typename First, typename... Rest>
constexpr auto gcd(First&& first, Rest&&... rest)
{
    using CommonType = Internals::SelectCommonType<First, Rest...>;

    if constexpr (Internals::is_duration<CommonType>::value) {
        using Dur = CommonType;
        using Rep = typename Dur::rep;

        static_assert(std::is_integral<Rep>::value,
                      "GCD for durations only makes sense if the representation is integral.");

        // Convert all arguments to CommonType (duration), collect counts
        Dur init = std::chrono::duration_cast<Dur>(std::forward<First>(first));
        Rep result = init.count();
        ((result = std::gcd(result, std::chrono::duration_cast<Dur>(std::forward<Rest>(rest)).count())), ...);

        return Dur{result};
    } else {
        // Non-duration case: assume integral or numeric types
        using CT = CommonType;
        CT result = static_cast<CT>(std::forward<First>(first));
        ((result = std::gcd(result, static_cast<CT>(std::forward<Rest>(rest)))), ...);
        return result;
    }
}

// Least common multiple
template <typename First, typename... Rest>
constexpr auto lcm(First&& first, Rest&&... rest)
{
    using CommonType = Internals::SelectCommonType<First, Rest...>;

    if constexpr (Internals::is_duration<CommonType>::value) {
        using Dur = CommonType;
        using Rep = typename Dur::rep;
        static_assert(std::is_integral<Rep>::value,
                      "LCM for durations only makes sense if the representation is integral.");

        Dur init = std::chrono::duration_cast<Dur>(std::forward<First>(first));
        Rep result = init.count();
        ((result = std::lcm(result, std::chrono::duration_cast<Dur>(std::forward<Rest>(rest)).count())), ...);

        return Dur{result};
    } else {
        // Non-duration case: assume integral or numeric types
        using CT = CommonType;
        CT result = static_cast<CT>(std::forward<First>(first));
        ((result = std::lcm(result, static_cast<CT>(std::forward<Rest>(rest)))), ...);
        return result;
    }
}

} // namespace utils_cpp
