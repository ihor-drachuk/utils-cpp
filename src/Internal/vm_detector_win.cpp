/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/utils-cpp
 * Contact:  ihor-drachuk-libs@pm.me  */

#include "vm_detector_platform.h"

#ifdef UTILS_CPP_OS_WINDOWS

#include <cstddef>
#include <vector>

#include <windows.h>

namespace utils_cpp {

namespace internal {

namespace {

std::vector<std::byte> readRawSmbiosData()
{
    constexpr DWORD kRawSmbiosTablesSignature = 'RSMB';

    const DWORD size = GetSystemFirmwareTable(kRawSmbiosTablesSignature, 0, nullptr, 0);
    if (size == 0)
        return {};

    std::vector<std::byte> buffer(size);
    const DWORD written = GetSystemFirmwareTable(kRawSmbiosTablesSignature, 0, buffer.data(), size);
    if (written > size)
        return {};

    buffer.resize(written);
    return buffer;
}

} // anonymous namespace

FirmwareInfo readFirmwareInfo()
{
    const auto table = smbiosTableAfterHeader(readRawSmbiosData());
    return parseSmbios(table.data(), table.size());
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
    return false;
}

} // namespace internal

} // namespace utils_cpp

#endif // UTILS_CPP_OS_WINDOWS
