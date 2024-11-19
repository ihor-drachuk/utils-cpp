/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/utils-cpp
 * Contact:  ihor-drachuk-libs@pm.me  */

#include <utils-cpp/vm_detector.h>

#include <utils-cpp/cpuid.h>
#include <utils-cpp/string_hash.h>

namespace utils_cpp {

// https://stackoverflow.com/a/66974571/5132939
std::optional<VM> getectVM()
{
    constexpr unsigned QueryVendorIdMagic = 0x40000000;

    // Upon execution, code should check bit 31 of register ECX
    // (the “hypervisor present bit”). If this bit is set, a hypervisor is present.
    // In a non-virtualized environment, the bit will be clear.
    if (!cpuid::getBit(1, cpuid::ecx, 31).value_or(false))
        return {};

    // A hypervisor is running on the machine. Query the vendor id.
    const auto optVendorId = cpuid::getStringRaw(QueryVendorIdMagic);
    if (!optVendorId)
        return {};

    switch (string_hash(optVendorId.value().data())) {
        case string_hash("VMwareVMware"): return VM::VMware;
        case string_hash("VBoxVBoxVBox"): return VM::VirtualBox;
        case string_hash("KVMKVMKVM"):    return VM::KVM;
        case string_hash("Microsoft Hv"): return VM::HyperV_VirtualPC; // Microsoft Hyper-V or Windows Virtual PC
        case string_hash("prl hyperv  "): return VM::Parallels;
        case string_hash("XenVMMXenVMM"): return VM::Xen;
        default:                          return VM::Unknown;
    }
}

} // namespace utils_cpp
