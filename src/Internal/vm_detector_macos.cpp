/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/utils-cpp
 * Contact:  ihor-drachuk-libs@pm.me  */

#include "vm_detector_platform.h"

#ifdef UTILS_CPP_OS_MACOS

#include "system_tools_macos_common.h"

namespace utils_cpp {

namespace internal {

FirmwareInfo readFirmwareInfo()
{
    FirmwareInfo info;
    info.systemManufacturer = get_iokit_platform_string_property("manufacturer").value_or(std::string {});
    info.systemProduct = get_sysctl_string("hw.model").value_or(std::string {});
    return info;
}

std::string readHypervisorType()
{
    return {};
}

bool readXenDom0()
{
    return false;
}

bool readOsHypervisorFlag()
{
    // The sysctl exists since macOS 11.3.
    return get_sysctl_int32("kern.hv_vmm_present").value_or(0) != 0;
}

} // namespace internal

} // namespace utils_cpp

#endif // UTILS_CPP_OS_MACOS
