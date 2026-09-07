/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/utils-cpp
 * Contact:  ihor-drachuk-libs@pm.me  */

#include "vm_detector_platform.h"

#ifdef UTILS_CPP_OS_LINUX

namespace utils_cpp {

namespace internal {

FirmwareInfo readFirmwareInfo()
{
    return readDmiFirmwareInfo("/sys/class/dmi/id");
}

std::string readHypervisorType()
{
    return readHypervisorTypeIn("/sys", "/proc");
}

bool readXenDom0()
{
    return readXenDom0In("/sys", "/proc");
}

bool readOsHypervisorFlag()
{
    return false;
}

} // namespace internal

} // namespace utils_cpp

#endif // UTILS_CPP_OS_LINUX
