/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/utils-cpp
 * Contact:  ihor-drachuk-libs@pm.me  */

#pragma once

#ifdef UTILS_CPP_OS_MACOS

#include <cstdint>
#include <optional>
#include <string>

namespace utils_cpp {

namespace internal {

std::optional<std::string> get_sysctl_string(const char* name);
std::optional<std::int32_t> get_sysctl_int32(const char* name);
std::optional<std::string> get_iokit_platform_string_property(const char* name);   // IOPlatformExpertDevice, CFString or CFData

} // namespace internal

} // namespace utils_cpp

#endif // UTILS_CPP_OS_MACOS
