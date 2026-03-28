/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/utils-cpp
 * Contact:  ihor-drachuk-libs@pm.me  */

#pragma once
#include <chrono>
#include <ostream>
#include <iomanip>

namespace utils_cpp_internals {

const char* durationUnit(std::chrono::nanoseconds*);
const char* durationUnit(std::chrono::microseconds*);
const char* durationUnit(std::chrono::milliseconds*);
const char* durationUnit(std::chrono::seconds*);
const char* durationUnit(std::chrono::minutes*);
const char* durationUnit(std::chrono::hours*);
const char* durationSeparator(std::chrono::nanoseconds*);
const char* durationSeparator(std::chrono::microseconds*);
const char* durationSeparator(std::chrono::milliseconds*);
const char* durationSeparator(std::chrono::seconds*);
const char* durationSeparator(std::chrono::minutes*);
const char* durationSeparator(std::chrono::hours*);

template <typename Dur, typename T1, typename T2>
void printDuration(const Dur& duration, T1, T2, std::ostream* os)
{
    const auto selector = static_cast<T1*>(nullptr);
    const auto total = std::chrono::duration_cast<T1>(duration);
    const auto leftover = std::chrono::duration_cast<T2>(duration - total);
    *os << total.count() << durationSeparator(selector)
        << std::setw(3) << std::setfill('0') << leftover.count()
        << ' '
        << durationUnit(selector);
}

template <typename Rep, typename Period>
void PrintTo(const std::chrono::duration<Rep, Period>& duration, std::ostream* os)
{
    using namespace std::chrono_literals;

    *os << "std::chrono::duration(";

    if (duration < 1us) {
        *os << std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count() << "ns";
    } else if (duration < 1ms) {
        utils_cpp_internals::printDuration(duration, 1us, 1ns, os);
    } else if (duration < 1s) {
        utils_cpp_internals::printDuration(duration, 1ms, 1us, os);
    } else if (duration < 1min) {
        utils_cpp_internals::printDuration(duration, 1s, 1ms, os);
    } else if (duration < 1h) {
        utils_cpp_internals::printDuration(duration, 1min, 1s, os);
    } else {
        utils_cpp_internals::printDuration(duration, 1h, 1min, os);
    }

    *os << ")";
}

} // namespace utils_cpp_internals

// codechecker_intentional [cert-dcl58-cpp]
namespace std::chrono {

template <typename Rep, typename Period>
void PrintTo(const std::chrono::duration<Rep, Period>& duration, std::ostream* os)
{
    utils_cpp_internals::PrintTo(duration, os);
}

} // namespace std::chrono
