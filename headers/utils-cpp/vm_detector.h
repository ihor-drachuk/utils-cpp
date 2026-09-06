/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/utils-cpp
 * Contact:  ihor-drachuk-libs@pm.me  */

#pragma once
#include <cstddef>
#include <optional>
#include <string>

namespace utils_cpp {

enum class VM {
    Unknown,
    VMware,
    VirtualBox,
    KVM,
    HyperV_VirtualPC,
    Parallels,
    Xen
};

struct SmbiosSystemInfo
{
    std::string manufacturer;
    std::string product;
};

struct VmEvidence
{
    bool hypervisorPresent {};      // CPUID leaf 1, ECX bit 31
    std::string hypervisorVendor;   // CPUID leaf 0x40000000, EBX:ECX:EDX, e.g. "Microsoft Hv"
    bool rootPartition {};          // CPUID leaf 0x40000003, EBX bit 0. Set only on the Hyper-V root partition.
    std::string smbiosManufacturer;
    std::string smbiosProduct;
};

// data starts after the RawSMBIOSData header, not at the beginning of the firmware table.
SmbiosSystemInfo parseSmbiosType1(const std::byte* data, std::size_t size);

VmEvidence collectVmEvidence();

// Priority when the evidence disagrees:
// - SMBIOS wins over the hypervisor vendor
// - the Hyper-V root partition reads as not a guest, because it is the host itself
std::optional<VM> classifyVm(const VmEvidence& evidence);

// SMBIOS decides. CPUID is consulted only when SMBIOS names nothing.
std::optional<VM> detectVm();

// CPUID only: what the hypervisor reports about itself. Blind to a hypervisor that hides its vendor leaf.
std::optional<VM> detectSupervisor();

// SMBIOS only: what the firmware says the machine is. Blind to a guest whose firmware mimics the host.
std::optional<VM> detectVmOnly();

} // namespace utils_cpp
