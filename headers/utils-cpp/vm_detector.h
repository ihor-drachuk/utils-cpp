/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/utils-cpp
 * Contact:  ihor-drachuk-libs@pm.me  */

#include <optional>

/* Notice! This module wasn't tested enough yet! */

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

// CPUID-based detection (may be masked by Hyper-V)
std::optional<VM> detectSupervisor();

// Non-CPUID detection (works around Hyper-V masking)
std::optional<VM> detectVmOnly();

} // namespace utils_cpp
