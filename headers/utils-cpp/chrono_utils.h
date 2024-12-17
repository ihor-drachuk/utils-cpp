/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/utils-cpp
 * Contact:  ihor-drachuk-libs@pm.me  */

#pragma once
#include <chrono>

namespace utils_cpp {

template<typename Rep, typename Period>
inline auto toHours(std::chrono::duration<Rep, Period> duration) { return std::chrono::duration_cast<std::chrono::hours>(duration); }

template<typename Rep, typename Period>
inline auto toMinutes(std::chrono::duration<Rep, Period> duration) { return std::chrono::duration_cast<std::chrono::minutes>(duration); }

template<typename Rep, typename Period>
inline auto toSeconds(std::chrono::duration<Rep, Period> duration) { return std::chrono::duration_cast<std::chrono::seconds>(duration); }

template<typename Rep, typename Period>
inline auto toMilliseconds(std::chrono::duration<Rep, Period> duration) { return std::chrono::duration_cast<std::chrono::milliseconds>(duration); }

} // namespace utils_cpp
