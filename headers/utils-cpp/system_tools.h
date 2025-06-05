/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/utils-cpp
 * Contact:  ihor-drachuk-libs@pm.me  */

#pragma once
#include <optional>
#include <string>
#include <utils-cpp/Internal/chassis_types.h>

namespace utils_cpp {

std::optional<bool> has_admin_rights();

std::optional<ChassisTypeMapping> get_chassis_type(); // Example: { ChassisTypeGeneralized::Desktop, ChassisTypeDetailed::MiniTower, "Mini Tower", "Desktop" }
std::optional<std::string> get_device_name();         // UTF8-encoded. Example: "My computer"

} // namespace utils_cpp
