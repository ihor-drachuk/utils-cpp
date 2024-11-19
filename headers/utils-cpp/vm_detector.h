/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/utils-cpp
 * Contact:  ihor-drachuk-libs@pm.me  */

#include <optional>

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

std::optional<VM> getectVM();

} // namespace utils_cpp
