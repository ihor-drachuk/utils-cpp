/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/utils-cpp
 * Contact:  ihor-drachuk-libs@pm.me  */

#pragma once

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

#include <utils-cpp/vm_detector.h>

namespace utils_cpp {

namespace internal {

FirmwareInfo readFirmwareInfo();
std::string readHypervisorType();   // empty outside Linux
bool readXenDom0();                 // false outside Linux
bool readOsHypervisorFlag();        // macOS kern.hv_vmm_present, false elsewhere

// The parsers and the Linux readers with their roots as parameters, built on every OS.
std::vector<std::byte> smbiosTableAfterHeader(std::vector<std::byte> rawSmbiosData);   // empty when too short
FirmwareInfo parseSmbios(const std::byte* data, std::size_t size);   // data starts after the RawSMBIOSData header
bool xenDom0FromFeatures(std::string_view featuresHex);              // /sys/hypervisor/properties/features
// Trailing whitespace and NULs are dropped, a missing file reads as empty.
std::string readTrimmedLine(const std::string& path);
FirmwareInfo readDmiFirmwareInfo(const std::string& dmiDir);              // the /sys/class/dmi/id layout
std::string readDeviceTreeHypervisor(const std::string& deviceTreeDir);   // hypervisor/compatible
std::string readHypervisorTypeIn(const std::string& sysfsRoot, const std::string& procRoot);
bool readXenDom0In(const std::string& sysfsRoot, const std::string& procRoot);

} // namespace internal

} // namespace utils_cpp
