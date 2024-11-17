/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/utils-cpp
 * Contact:  ihor-drachuk-libs@pm.me  */

#include <utils-cpp/gtest_printers.h>

namespace utils_cpp_internals {

const char* durationUnit(std::chrono::nanoseconds*) { return "ns"; }
const char* durationUnit(std::chrono::microseconds*) { return "us"; }
const char* durationUnit(std::chrono::milliseconds*) { return "ms"; }
const char* durationUnit(std::chrono::seconds*) { return "s"; }
const char* durationUnit(std::chrono::minutes*) { return "m"; }
const char* durationUnit(std::chrono::hours*) { return "h"; }
const char* durationSeparator(std::chrono::nanoseconds*) { return "."; }
const char* durationSeparator(std::chrono::microseconds*) { return "."; }
const char* durationSeparator(std::chrono::milliseconds*) { return "."; }
const char* durationSeparator(std::chrono::seconds*) { return "."; }
const char* durationSeparator(std::chrono::minutes*) { return ":"; }
const char* durationSeparator(std::chrono::hours*) { return ":"; }

} // namespace utils_cpp_internals
