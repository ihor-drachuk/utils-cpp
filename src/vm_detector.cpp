/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/utils-cpp
 * Contact:  ihor-drachuk-libs@pm.me  */

#include <utils-cpp/vm_detector.h>

#include <string_view>

#include <utils-cpp/cpuid.h>

#include "Internal/vm_detector_platform.h"

namespace {

using utils_cpp::FirmwareInfo;
using utils_cpp::VM;

constexpr utils_cpp::cpuid::Reg32 kHypervisorVendorLeaf = 0x40000000;
constexpr utils_cpp::cpuid::Reg32 kHypervisorFeaturesLeaf = 0x40000003;
constexpr utils_cpp::cpuid::Reg32 kHypervisorRecommendationsLeaf = 0x40000004;
constexpr utils_cpp::cpuid::Reg32 kNestedHypervisorVendorLeaf = 0x40000100;
constexpr unsigned kHypervisorPresentBit = 31;
constexpr unsigned kCreatePartitionsBit = 0;
constexpr unsigned kNestedInHyperVBit = 12;

constexpr std::string_view kMicrosoftHv = "Microsoft Hv";
constexpr std::string_view kKvmSignature = "KVMKVMKVM";
constexpr std::string_view kAmazonEc2 = "Amazon EC2";
constexpr std::string_view kGoogleComputeEngine = "Google Compute Engine";

struct VendorPrefix
{
    std::string_view prefix;
    VM vm;
};

// Vendors whose guests may also carry another hypervisor's signature: nested Hyper-V inside VMware, Xen cloaked as
// Hyper-V. Their firmware strings are trusted before any signature.
constexpr VendorPrefix kMasqueradedVendors[] = {
    {"VMware", VM::VMware},
    {"VMW", VM::VMware},
    {"innotek GmbH", VM::VirtualBox},
    {"VirtualBox", VM::VirtualBox},
    {"Parallels", VM::Parallels},
    {"Xen", VM::Xen},
    {"Hyper-V", VM::HyperV_VirtualPC},
    {"Apple Virtualization", VM::Apple},
    {"VirtualMac", VM::Apple},
    {"BHYVE", VM::Bhyve},
};

// Firmware that fronts more than one hypervisor (QEMU also runs without KVM, a cloud may run on ESXi), so the
// hypervisor signature is consulted first.
constexpr VendorPrefix kPlainVendors[] = {
    {"KVM", VM::KVM},
    {"OpenStack", VM::KVM},
    {"KubeVirt", VM::KVM},
    {"Alibaba Cloud ECS", VM::KVM},
    {"QEMU", VM::QEMU},
    {"Bochs", VM::QEMU},
};

constexpr VendorPrefix kHypervisorSignatures[] = {
    {"XenVMMXenVMM", VM::Xen},
    {"KVMKVMKVM", VM::KVM},
    {"Linux KVM Hv", VM::KVM},
    {"TCGTCGTCGTCG", VM::QEMU},
    {"VMwareVMware", VM::VMware},
    {"VBoxVBoxVBox", VM::VirtualBox},
    {"bhyve bhyve", VM::Bhyve},
    {"Apple VZ", VM::Apple},
    {" lrpepyh vr", VM::Parallels},
    {"prl hyperv", VM::Parallels},
};

bool startsWith(std::string_view text, std::string_view prefix)
{
    return text.substr(0, prefix.size()) == prefix;
}

bool contains(std::string_view text, std::string_view needle)
{
    return text.find(needle) != std::string_view::npos;
}

std::string withoutTrailingPadding(std::string_view text)
{
    const auto end = text.find_last_not_of(std::string_view(" \0", 2));
    return std::string(end == std::string_view::npos ? std::string_view {} : text.substr(0, end + 1));
}

struct FirmwareFields
{
    std::string_view fields[5];

    FirmwareFields(const FirmwareInfo& firmware)
        : fields {firmware.systemProduct,
                  firmware.systemManufacturer,
                  firmware.boardManufacturer,
                  firmware.biosVendor,
                  firmware.productVersion}
    {}

    bool anyStartsWith(std::string_view prefix) const
    {
        for (const auto& field : fields)
            if (startsWith(field, prefix))
                return true;
        return false;
    }

    template<std::size_t N>
    std::optional<VM> firstMatch(const VendorPrefix (&vendors)[N]) const
    {
        for (const auto& field : fields)
            for (const auto& vendor : vendors)
                if (startsWith(field, vendor.prefix))
                    return vendor.vm;
        return {};
    }
};

// An EC2 bare-metal instance type ends in ".metal" or continues with a size after it.
bool isEc2BareMetal(std::string_view product)
{
    const auto pos = product.find(".metal");
    if (pos == std::string_view::npos)
        return false;
    const auto rest = product.substr(pos + 6);
    return rest.empty() || rest.front() == '-';
}

std::optional<VM> vmFromMasqueradedFirmware(const FirmwareInfo& firmware, bool cpuidAvailable)
{
    const FirmwareFields fields(firmware);

    if (startsWith(firmware.systemManufacturer, "Microsoft Corporation") && startsWith(firmware.systemProduct, "Virtual Machine"))
        return VM::HyperV_VirtualPC;

    if (fields.anyStartsWith(kAmazonEc2) && !isEc2BareMetal(firmware.systemProduct))
        return VM::KVM;

    // Bare-metal GCE machines carry the same string, so on x86 the KVM signature has to confirm it.
    if (!cpuidAvailable && fields.anyStartsWith(kGoogleComputeEngine))
        return VM::KVM;

    return fields.firstMatch(kMasqueradedVendors);
}

std::optional<VM> vmFromHypervisorType(std::string_view type)
{
    if (startsWith(type, "xen"))
        return VM::Xen;
    if (startsWith(type, "linux,kvm"))
        return VM::KVM;
    if (contains(type, "vmware"))
        return VM::VMware;
    if (type.empty())
        return {};
    return VM::Unknown;
}

std::optional<VM> vmFromHypervisorSignature(std::string_view vendor)
{
    for (const auto& signature : kHypervisorSignatures)
        if (vendor == signature.prefix)
            return signature.vm;
    if (vendor.empty())
        return {};
    return VM::Unknown;
}

} // anonymous namespace

namespace utils_cpp {

VmEvidence collectVmEvidence()
{
    VmEvidence evidence;

    evidence.cpuidAvailable = cpuid::get(0).has_value();
    const bool cpuidHypervisorPresent = cpuid::getBit(1, cpuid::ecx, kHypervisorPresentBit).value_or(false);
    evidence.hypervisorPresent = cpuidHypervisorPresent || internal::readOsHypervisorFlag();

    if (cpuidHypervisorPresent) {
        if (const auto optVendor = cpuid::getStringRaw(kHypervisorVendorLeaf))
            evidence.hypervisorVendor = withoutTrailingPadding(optVendor->data());

        if (evidence.hypervisorVendor == kMicrosoftHv) {
            evidence.hyperVRootPartition = cpuid::getBit(kHypervisorFeaturesLeaf, cpuid::ebx, kCreatePartitionsBit)
                                               .value_or(false);
            evidence.hyperVNested = cpuid::getBit(kHypervisorRecommendationsLeaf, cpuid::eax, kNestedInHyperVBit)
                                        .value_or(false);
            if (const auto optNestedVendor = cpuid::getStringRaw(kNestedHypervisorVendorLeaf))
                evidence.nestedHypervisorVendor = withoutTrailingPadding(optNestedVendor->data());
        }
    }

    evidence.hypervisorType = internal::readHypervisorType();
    evidence.xenDom0 = internal::readXenDom0();
    evidence.firmware = internal::readFirmwareInfo();

    return evidence;
}

std::optional<VM> classifyVm(const VmEvidence& evidence)
{
    if (const auto optVm = vmFromMasqueradedFirmware(evidence.firmware, evidence.cpuidAvailable))
        return optVm;

    // Unknown stays deferred: the plain firmware stage may still name the vendor.
    std::optional<VM> deferred;

    // The control domain's own Xen shows up in hypervisorType and as the Xen signature.
    if (!evidence.xenDom0) {
        if (const auto optVm = vmFromHypervisorType(evidence.hypervisorType)) {
            if (*optVm != VM::Unknown)
                return optVm;
            deferred = VM::Unknown;
        }
    }

    if (evidence.hypervisorVendor == kMicrosoftHv) {
        if (evidence.nestedHypervisorVendor == kKvmSignature)
            return VM::KVM;
        // A host running Hyper-V, VBS or WSL2 reports "Microsoft Hv" from its own root partition.
        if (evidence.hyperVRootPartition && !evidence.hyperVNested)
            return {};
        // QEMU reports "Microsoft Hv" too when it provides Hyper-V enlightenments, so firmware gets a say first.
        deferred = VM::HyperV_VirtualPC;
    } else if (const auto optVm = vmFromHypervisorSignature(evidence.hypervisorVendor)) {
        const bool ownXen = evidence.xenDom0 && *optVm == VM::Xen;
        if (!ownXen && *optVm != VM::Unknown)
            return optVm;
        if (!ownXen)
            deferred = VM::Unknown;
    }

    // Only an outer hypervisor, caught above or still deferred, makes the control domain a guest.
    if (evidence.xenDom0)
        return deferred;

    if (const auto optVm = FirmwareFields(evidence.firmware).firstMatch(kPlainVendors))
        return optVm;

    if (deferred)
        return deferred;

    if (evidence.hypervisorPresent)
        return VM::Unknown;

    return {};
}

std::optional<VM> detectVm()
{
    return classifyVm(collectVmEvidence());
}

} // namespace utils_cpp
