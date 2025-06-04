/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/utils-cpp
 * Contact:  ihor-drachuk-libs@pm.me  */

#include "system_tools_common.h"

#include <utils-cpp/Internal/chassis_types.h>

#ifdef UTILS_CPP_OS_LINUX

#include <fstream>
#include <string>
#include <unistd.h>
#include <limits.h>

namespace utils_cpp {

namespace {

std::optional<std::string> read_dmi_file(const std::string& filename)
{
    std::ifstream file("/sys/class/dmi/id/" + filename);
    if (!file.is_open())
        return {};

    std::string content;
    std::getline(file, content);

    if (content.empty())
        return {};

    return {content};
}

std::optional<int> read_dmi_integer(const std::string& filename)
{
    const auto content = read_dmi_file(filename);
    if (!content)
        return {};

    try {
        return std::stoi(*content);
    } catch (...) {
        return {};
    }
}

} // namespace

namespace internal {

std::optional<ChassisTypeMapping> get_chassis_type()
{
    const auto optChassisType = read_dmi_integer("chassis_type");
    if (!optChassisType)
        return {};

    return get_chassis_by_int(*optChassisType);
}

std::optional<std::string> get_device_name()
{
    char hostname[HOST_NAME_MAX + 1];

    if (gethostname(hostname, sizeof(hostname)) != 0)
        return {};

    // Ensure null termination
    hostname[HOST_NAME_MAX] = '\0';

    return std::string(hostname);
}

} // namespace internal

} // namespace utils_cpp

#endif // UTILS_CPP_OS_LINUX
