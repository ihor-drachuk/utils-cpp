/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/utils-cpp
 * Contact:  ihor-drachuk-libs@pm.me  */

#pragma once

#include <utils-cpp/system_tools.h>

namespace utils_cpp {

namespace internal {

std::optional<ChassisTypeMapping> get_chassis_type();
std::optional<std::string> get_device_name();

} // namespace internal

} // namespace utils_cpp
