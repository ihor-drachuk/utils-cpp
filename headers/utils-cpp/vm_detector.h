/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/utils-cpp
 * Contact:  ihor-drachuk-libs@pm.me  */

#pragma once
#include <optional>
#include <string>

namespace utils_cpp {

enum class VM {
    Unknown,            // a hypervisor is present, but its vendor is not recognized
    VMware,
    VirtualBox,
    KVM,                // also the clouds built on it: EC2 Nitro, Google Compute Engine, OpenStack, KubeVirt, Alibaba Cloud
    HyperV_VirtualPC,   // also Azure
    Parallels,
    Xen,
    QEMU,               // QEMU without KVM, or QEMU firmware with no usable hypervisor signature
    Bhyve,
    Apple               // Apple Virtualization framework: every macOS guest on Apple silicon, Parallels and Fusion included
};

// Windows: SMBIOS types 0, 1 and 2. Linux: /sys/class/dmi/id. macOS: IOPlatformExpertDevice and hw.model only.
struct FirmwareInfo
{
    std::string biosVendor;
    std::string systemManufacturer;
    std::string systemProduct;
    std::string productVersion;
    std::string boardManufacturer;
};

struct VmEvidence
{
    bool cpuidAvailable {};              // false on non-x86 builds, where the GCE firmware string decides without the KVM signature
    bool hypervisorPresent {};           // x86: CPUID leaf 1, ECX bit 31. macOS: also kern.hv_vmm_present
    std::string hypervisorVendor;        // x86: CPUID leaf 0x40000000, e.g. "Microsoft Hv"
    bool hyperVRootPartition {};         // CPUID leaf 0x40000003, EBX bit 0 (CreatePartitions). Read under "Microsoft Hv" only
    bool hyperVNested {};                // CPUID leaf 0x40000004, EAX bit 12. Read under "Microsoft Hv" only
    std::string nestedHypervisorVendor;  // CPUID leaf 0x40000100. QEMU keeps "KVMKVMKVM" here behind Hyper-V enlightenments
    std::string hypervisorType;          // Linux: /sys/hypervisor/type, "xen" with /proc/xen/capabilities, else the device tree
    bool xenDom0 {};                     // Linux: the Xen control domain, which is the host
    FirmwareInfo firmware;
};

VmEvidence collectVmEvidence();

// The first stage that names a vendor wins:
// - firmware strings of the vendors whose guests may carry another hypervisor's signature
//   (VMware, VirtualBox, Parallels, Xen, Hyper-V, Apple, bhyve, Amazon EC2, and Google Compute Engine without CPUID)
// - hypervisorType, then hypervisorVendor
// - the remaining firmware strings (QEMU, Bochs, the KVM clouds)
// The host itself reads as no VM unless an earlier stage named an outer hypervisor: the Xen control domain, and the
// Hyper-V root partition that is not itself nested. An unrecognized hypervisor reads as VM::Unknown.
std::optional<VM> classifyVm(const VmEvidence& evidence);

std::optional<VM> detectVm();

} // namespace utils_cpp
