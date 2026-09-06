/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/utils-cpp
 * Contact:  ihor-drachuk-libs@pm.me  */

#include <utils-cpp/vm_detector.h>

#include <utils-cpp/cpuid.h>

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <string_view>
#include <vector>

#ifdef UTILS_CPP_OS_WINDOWS
#include <windows.h>
#endif // UTILS_CPP_OS_WINDOWS

namespace {

constexpr utils_cpp::cpuid::Reg32 kHypervisorVendorLeaf = 0x40000000;
constexpr utils_cpp::cpuid::Reg32 kHypervisorFeaturesLeaf = 0x40000003;
constexpr unsigned kHypervisorPresentBit = 31;
constexpr unsigned kCreatePartitionsBit = 0;

bool containsIgnoreCase(std::string_view text, std::string_view needle)
{
    const auto it = std::search(text.begin(), text.end(), needle.begin(), needle.end(),
                                [](unsigned char a, unsigned char b) { return std::tolower(a) == std::tolower(b); });
    return it != text.end();
}

std::optional<utils_cpp::VM> vmFromSmbios(const std::string& manufacturer, const std::string& product)
{
    if (containsIgnoreCase(manufacturer, "VMware") || containsIgnoreCase(product, "VMware"))
        return utils_cpp::VM::VMware;
    if (containsIgnoreCase(manufacturer, "innotek") || containsIgnoreCase(product, "VirtualBox"))
        return utils_cpp::VM::VirtualBox;
    if (containsIgnoreCase(manufacturer, "Parallels") || containsIgnoreCase(product, "Parallels"))
        return utils_cpp::VM::Parallels;
    if (containsIgnoreCase(manufacturer, "QEMU") || containsIgnoreCase(product, "KVM"))
        return utils_cpp::VM::KVM;
    if (containsIgnoreCase(manufacturer, "Xen") || containsIgnoreCase(product, "HVM domU"))
        return utils_cpp::VM::Xen;
    if (containsIgnoreCase(manufacturer, "Microsoft Corporation") && containsIgnoreCase(product, "Virtual Machine"))
        return utils_cpp::VM::HyperV_VirtualPC;
    return {};
}

std::optional<utils_cpp::VM> vmFromHypervisorVendor(const std::string& vendor)
{
    if (vendor == "VMwareVMware")
        return utils_cpp::VM::VMware;
    if (vendor == "VBoxVBoxVBox")
        return utils_cpp::VM::VirtualBox;
    if (vendor == "KVMKVMKVM")
        return utils_cpp::VM::KVM;
    if (vendor == "Microsoft Hv")
        return utils_cpp::VM::HyperV_VirtualPC;
    if (vendor == " lrpepyh vr" || vendor == "prl hyperv  ")
        return utils_cpp::VM::Parallels;
    if (vendor == "XenVMMXenVMM")
        return utils_cpp::VM::Xen;
    return utils_cpp::VM::Unknown;
}

#ifdef UTILS_CPP_OS_WINDOWS

std::vector<std::byte> readSmbiosTable()
{
    constexpr DWORD kRawSmbiosTablesSignature = 'RSMB';
    // RawSMBIOSData: Used20CallingMethod, SMBIOSMajorVersion, SMBIOSMinorVersion, DmiRevision, Length.
    constexpr DWORD kRawSmbiosDataHeaderSize = 8;

    const DWORD size = GetSystemFirmwareTable(kRawSmbiosTablesSignature, 0, nullptr, 0);
    if (size <= kRawSmbiosDataHeaderSize)
        return {};

    std::vector<std::byte> buffer(size);
    const DWORD written = GetSystemFirmwareTable(kRawSmbiosTablesSignature, 0, buffer.data(), size);
    if (written == 0 || written > size)
        return {};

    buffer.resize(written);
    buffer.erase(buffer.begin(), buffer.begin() + kRawSmbiosDataHeaderSize);
    return buffer;
}

#else // Not Windows

std::vector<std::byte> readSmbiosTable()
{
    return {};
}

#endif // UTILS_CPP_OS_WINDOWS

} // anonymous namespace

namespace utils_cpp {

SmbiosSystemInfo parseSmbiosType1(const std::byte* data, std::size_t size)
{
    constexpr std::uint8_t kTypeSystemInformation = 1;
    constexpr std::uint8_t kTypeEndOfTable = 127;
    constexpr std::size_t kHeaderSize = 4;              // type, length, handle
    constexpr std::size_t kManufacturerOffset = 0x04;   // string index
    constexpr std::size_t kProductOffset = 0x05;        // string index

    if (!data)
        return {};

    const auto at = [&](std::size_t index) { return static_cast<std::uint8_t>(data[index]); };

    std::size_t pos {};
    while (pos + kHeaderSize <= size) {
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

        if (type == kTypeSystemInformation) {
            const auto stringAt = [&](std::size_t offset) -> std::string {
                if (offset >= length)
                    return {};
                const std::uint8_t index = at(pos + offset);
                return index >= 1 && index <= strings.size() ? strings[index - 1] : std::string {};
            };
            return {stringAt(kManufacturerOffset), stringAt(kProductOffset)};
        }

        if (next <= pos)
            break;
        pos = next;
    }

    return {};
}

VmEvidence collectVmEvidence()
{
    VmEvidence evidence;

    evidence.hypervisorPresent = cpuid::getBit(1, cpuid::ecx, kHypervisorPresentBit).value_or(false);

    if (evidence.hypervisorPresent) {
        if (const auto optVendor = cpuid::getStringRaw(kHypervisorVendorLeaf))
            evidence.hypervisorVendor = std::string(optVendor->data());

        evidence.rootPartition = cpuid::getBit(kHypervisorFeaturesLeaf, cpuid::ebx, kCreatePartitionsBit)
                                     .value_or(false);
    }

    const auto table = readSmbiosTable();
    const auto system = parseSmbiosType1(table.data(), table.size());
    evidence.smbiosManufacturer = system.manufacturer;
    evidence.smbiosProduct = system.product;

    return evidence;
}

std::optional<VM> classifyVm(const VmEvidence& evidence)
{
    if (const auto optVm = vmFromSmbios(evidence.smbiosManufacturer, evidence.smbiosProduct))
        return optVm;

    if (!evidence.hypervisorPresent)
        return {};

    // A host running Hyper-V, VBS or WSL2 reports "Microsoft Hv" from its own root partition.
    if (evidence.hypervisorVendor == "Microsoft Hv" && evidence.rootPartition)
        return {};

    return vmFromHypervisorVendor(evidence.hypervisorVendor);
}

std::optional<VM> detectVm()
{
    return classifyVm(collectVmEvidence());
}

std::optional<VM> detectSupervisor()
{
    auto evidence = collectVmEvidence();
    // Blanking the SMBIOS fields makes classifyVm fall through to the CPUID vendor.
    evidence.smbiosManufacturer.clear();
    evidence.smbiosProduct.clear();
    return classifyVm(evidence);
}

std::optional<VM> detectVmOnly()
{
    const auto table = readSmbiosTable();
    const auto system = parseSmbiosType1(table.data(), table.size());
    return vmFromSmbios(system.manufacturer, system.product);
}

} // namespace utils_cpp
