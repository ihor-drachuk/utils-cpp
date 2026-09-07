/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/utils-cpp
 * Contact:  ihor-drachuk-libs@pm.me  */

#include "vm_detector_platform.h"

#include <cstdint>
#include <exception>
#include <fstream>
#include <string>
#include <string_view>
#include <vector>

namespace utils_cpp {

namespace internal {

std::vector<std::byte> smbiosTableAfterHeader(std::vector<std::byte> rawSmbiosData)
{
    // RawSMBIOSData: Used20CallingMethod, SMBIOSMajorVersion, SMBIOSMinorVersion, DmiRevision, Length.
    constexpr std::size_t kRawSmbiosDataHeaderSize = 8;

    if (rawSmbiosData.size() <= kRawSmbiosDataHeaderSize)
        return {};

    rawSmbiosData.erase(rawSmbiosData.begin(), rawSmbiosData.begin() + kRawSmbiosDataHeaderSize);
    return rawSmbiosData;
}

bool xenDom0FromFeatures(std::string_view featuresHex)
{
    constexpr unsigned kXenFeatDom0 = 11;

    unsigned long features {};
    try {
        features = std::stoul(std::string(featuresHex), nullptr, 16);
    } catch (const std::exception&) {
        return false;
    }
    return (features & (1ul << kXenFeatDom0)) != 0;
}

FirmwareInfo parseSmbios(const std::byte* data, std::size_t size)
{
    constexpr std::uint8_t kTypeBiosInformation = 0;
    constexpr std::uint8_t kTypeSystemInformation = 1;
    constexpr std::uint8_t kTypeBaseboardInformation = 2;
    constexpr std::uint8_t kTypeEndOfTable = 127;
    constexpr std::size_t kHeaderSize = 4;   // type, length, handle
    // String indices within the formatted area
    constexpr std::size_t kBiosVendorOffset = 0x04;
    constexpr std::size_t kManufacturerOffset = 0x04;
    constexpr std::size_t kProductOffset = 0x05;
    constexpr std::size_t kVersionOffset = 0x06;
    constexpr std::size_t kBoardManufacturerOffset = 0x04;

    FirmwareInfo info;
    if (!data)
        return info;

    const auto at = [&](std::size_t index) { return static_cast<std::uint8_t>(data[index]); };

    bool biosSeen {};
    bool systemSeen {};
    bool boardSeen {};
    std::size_t pos {};

    while (pos + kHeaderSize <= size && !(biosSeen && systemSeen && boardSeen)) {
        const std::uint8_t type = at(pos);
        const std::uint8_t length = at(pos + 1);
        if (type == kTypeEndOfTable || length < kHeaderSize || pos + length > size)
            break;

        // The formatted area is followed by the string set: NUL-terminated strings, then one more NUL. A record
        // with no strings carries the two NUL bytes on their own, so the record always ends on a NUL pair.
        std::vector<std::string> strings;
        std::size_t cursor = pos + length;
        while (cursor < size && at(cursor) != 0) {
            std::size_t end = cursor;
            while (end < size && at(end) != 0)
                ++end;
            strings.emplace_back(reinterpret_cast<const char*>(data + cursor), end - cursor);
            cursor = end < size ? end + 1 : end;
        }
        // A stringless record ends on two NULs, so skipping the set costs one byte more than after the last string.
        const std::size_t next = strings.empty() ? cursor + 2 : cursor + 1;

        const auto stringAt = [&](std::size_t offset) -> std::string {
            if (offset >= length)
                return {};
            const std::uint8_t index = at(pos + offset);
            return index >= 1 && index <= strings.size() ? strings[index - 1] : std::string {};
        };

        if (type == kTypeBiosInformation && !biosSeen) {
            info.biosVendor = stringAt(kBiosVendorOffset);
            biosSeen = true;
        } else if (type == kTypeSystemInformation && !systemSeen) {
            info.systemManufacturer = stringAt(kManufacturerOffset);
            info.systemProduct = stringAt(kProductOffset);
            info.productVersion = stringAt(kVersionOffset);
            systemSeen = true;
        } else if (type == kTypeBaseboardInformation && !boardSeen) {
            info.boardManufacturer = stringAt(kBoardManufacturerOffset);
            boardSeen = true;
        }

        if (next <= pos)
            break;
        pos = next;
    }

    return info;
}

std::string readTrimmedLine(const std::string& path)
{
    std::ifstream file(path);
    std::string line;
    if (!file || !std::getline(file, line))
        return {};

    const auto end = line.find_last_not_of(std::string_view(" \t\r\n\0", 5));
    line.erase(end == std::string::npos ? 0 : end + 1);
    return line;
}

FirmwareInfo readDmiFirmwareInfo(const std::string& dmiDir)
{
    FirmwareInfo info;
    info.biosVendor = readTrimmedLine(dmiDir + "/bios_vendor");
    info.systemManufacturer = readTrimmedLine(dmiDir + "/sys_vendor");
    info.systemProduct = readTrimmedLine(dmiDir + "/product_name");
    info.productVersion = readTrimmedLine(dmiDir + "/product_version");
    info.boardManufacturer = readTrimmedLine(dmiDir + "/board_vendor");
    return info;
}

std::string readDeviceTreeHypervisor(const std::string& deviceTreeDir)
{
    return readTrimmedLine(deviceTreeDir + "/hypervisor/compatible");
}

std::string readHypervisorTypeIn(const std::string& sysfsRoot, const std::string& procRoot)
{
    if (auto type = readTrimmedLine(sysfsRoot + "/hypervisor/type"); !type.empty())
        return type;

    if (std::ifstream(procRoot + "/xen/capabilities"))
        return "xen";

    return readDeviceTreeHypervisor(procRoot + "/device-tree");
}

bool readXenDom0In(const std::string& sysfsRoot, const std::string& procRoot)
{
    // /proc/xen is mounted late in boot, so the sysfs feature mask is asked first.
    if (const auto features = readTrimmedLine(sysfsRoot + "/hypervisor/properties/features"); !features.empty())
        return xenDom0FromFeatures(features);

    return readTrimmedLine(procRoot + "/xen/capabilities").find("control_d") != std::string::npos;
}

} // namespace internal

} // namespace utils_cpp
