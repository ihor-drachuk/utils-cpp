/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/utils-cpp
 * Contact:  ihor-drachuk-libs@pm.me  */

#include <utils-cpp/vm_detector.h>

#include <utils-cpp/cpuid.h>
#include <utils-cpp/string_hash.h>

#include <algorithm>
#include <string>
#include <cctype>
#include <memory>

#ifdef UTILS_CPP_OS_WINDOWS
#include <windows.h>
#include <wbemidl.h>
#include <comdef.h>
#include <setupapi.h>
#include <tlhelp32.h>
#include <iphlpapi.h>
#elif defined(UTILS_CPP_OS_LINUX)
#include <fstream>
#include <dirent.h>
#include <sys/stat.h>
#include <cstring>
#elif defined(UTILS_CPP_OS_MACOS)
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <sys/sysctl.h>
#include <libproc.h>
#include <ifaddrs.h>
#include <net/if_dl.h>
#include <cstring>
#include <vector>
#endif

namespace {

// Safe character transformation functions
inline char safe_tolower(char c) {
    return static_cast<char>(::tolower(static_cast<unsigned char>(c)));
}

inline wchar_t safe_towlower(wchar_t c) { // NOLINT(clang-diagnostic-unused-function)
    return ::towlower(c);
}

inline bool safe_isdigit(char c) {
    return ::isdigit(static_cast<unsigned char>(c)) != 0;
}

// Generic RAII cleaner (used by Windows and macOS)
template<typename F>
auto make_cleaner(F&& f) {
    auto deleter = [f = std::forward<F>(f)](void*){ f(); };
    return std::unique_ptr<void, decltype(deleter)>(reinterpret_cast<void*>(1), std::move(deleter));
}

#ifdef UTILS_CPP_OS_MACOS
// RAII wrapper for CFTypeRef
template<typename T>
class CFPtr {
public:
    explicit CFPtr(T ref = nullptr) : ref_(ref) {}
    ~CFPtr() { if (ref_) CFRelease(ref_); }

    CFPtr(const CFPtr&) = delete;
    CFPtr& operator=(const CFPtr&) = delete;

    CFPtr(CFPtr&& other) noexcept : ref_(other.ref_) { other.ref_ = nullptr; }
    CFPtr& operator=(CFPtr&& other) noexcept {
        if (this != &other) {
            if (ref_) CFRelease(ref_);
            ref_ = other.ref_;
            other.ref_ = nullptr;
        }
        return *this;
    }

    T get() const { return ref_; }
    T* getRef() { return &ref_; }
    operator bool() const { return ref_ != nullptr; }

private:
    T ref_;
};

// RAII wrapper for ifaddrs
class IfAddrsPtr {
public:
    explicit IfAddrsPtr(struct ifaddrs* ptr = nullptr) : ptr_(ptr) {}
    ~IfAddrsPtr() { if (ptr_) freeifaddrs(ptr_); }

    IfAddrsPtr(const IfAddrsPtr&) = delete;
    IfAddrsPtr& operator=(const IfAddrsPtr&) = delete;

    IfAddrsPtr(IfAddrsPtr&& other) noexcept : ptr_(other.ptr_) { other.ptr_ = nullptr; }
    IfAddrsPtr& operator=(IfAddrsPtr&& other) noexcept {
        if (this != &other) {
            if (ptr_) freeifaddrs(ptr_);
            ptr_ = other.ptr_;
            other.ptr_ = nullptr;
        }
        return *this;
    }

    struct ifaddrs* get() const { return ptr_; }
    struct ifaddrs** getRef() { return &ptr_; }
    operator bool() const { return ptr_ != nullptr; }

private:
    struct ifaddrs* ptr_;
};
#endif

} // anonymous namespace

namespace utils_cpp {

// https://stackoverflow.com/a/66974571/5132939
std::optional<VM> detectSupervisor()
{
    constexpr unsigned QueryVendorIdMagic = 0x40000000;

    // Upon execution, code should check bit 31 of register ECX
    // (the "hypervisor present bit"). If this bit is set, a hypervisor is present.
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

std::optional<VM> detectVmOnly()
{
#ifdef UTILS_CPP_OS_WINDOWS

    // Method 1: Device/Driver detection (highest priority - indicates currently loaded drivers)
    do {
        HDEVINFO deviceInfoSet = SetupDiGetClassDevs(nullptr, nullptr, nullptr, DIGCF_ALLCLASSES | DIGCF_PRESENT);
        if (deviceInfoSet == INVALID_HANDLE_VALUE) break;

        auto cleanup = make_cleaner([deviceInfoSet]() {
            SetupDiDestroyDeviceInfoList(deviceInfoSet);
        });

        SP_DEVINFO_DATA deviceInfoData;
        deviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

        for (DWORD i = 0; SetupDiEnumDeviceInfo(deviceInfoSet, i, &deviceInfoData); i++) {
            char deviceDesc[256];
            if (SetupDiGetDeviceRegistryPropertyA(deviceInfoSet, &deviceInfoData, SPDRP_DEVICEDESC,
                                                nullptr, (PBYTE)deviceDesc, sizeof(deviceDesc), nullptr)) {
                std::string desc = deviceDesc;
                std::transform(desc.begin(), desc.end(), desc.begin(), safe_tolower);

                // VMware guest-specific devices (not host-side VMware installations)
                if (desc.find("vmware svga") != std::string::npos ||
                    desc.find("vmware scsi") != std::string::npos ||
                    desc.find("vmware accelerated") != std::string::npos ||
                    desc.find("vmxnet") != std::string::npos ||
                    desc.find("vmware tools") != std::string::npos) {
                    return VM::VMware;
                }

                // VirtualBox guest-specific devices
                if (desc.find("vboxguest") != std::string::npos ||
                    desc.find("virtualbox guest") != std::string::npos ||
                    desc.find("vbox guest") != std::string::npos ||
                    desc.find("vboxvideo") != std::string::npos ||
                    desc.find("vboxmouse") != std::string::npos) {
                    return VM::VirtualBox;
                }

                // Parallels guest-specific devices
                if (desc.find("parallels display") != std::string::npos ||
                    desc.find("parallels tools") != std::string::npos ||
                    desc.find("prl ") != std::string::npos) {
                    return VM::Parallels;
                }
            }
        }
    } while (false);

    // Method 2: WMI-based hardware detection (high priority - current system info)
    do {
        HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);
        bool comInitialized = SUCCEEDED(hr);
        auto comCleanup = make_cleaner([comInitialized]() {
            if (comInitialized) {
                CoUninitialize();
            }
        });

        if (!comInitialized && hr != RPC_E_CHANGED_MODE) break;

        IWbemLocator* pLoc = nullptr;
        hr = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID*)&pLoc);
        if (FAILED(hr)) break;

        auto locCleanup = make_cleaner([pLoc]() { pLoc->Release(); });

        IWbemServices* pSvc = nullptr;
        hr = pLoc->ConnectServer(_bstr_t(L"ROOT\\CIMV2"), nullptr, nullptr, 0, 0, 0, 0, &pSvc);
        if (FAILED(hr)) break;

        auto svcCleanup = make_cleaner([pSvc]() { pSvc->Release(); });

        hr = CoSetProxyBlanket(pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, nullptr,
                             RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, EOAC_NONE);
        if (FAILED(hr)) break;

        IEnumWbemClassObject* pEnumerator = nullptr;
        hr = pSvc->ExecQuery(_bstr_t(L"WQL"),
                           _bstr_t(L"SELECT Manufacturer, Model FROM Win32_ComputerSystem"),
                           WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
                           nullptr, &pEnumerator);
        if (FAILED(hr)) break;

        auto enumCleanup = make_cleaner([pEnumerator]() { pEnumerator->Release(); });

        IWbemClassObject* pclsObj = nullptr;
        ULONG uReturn = 0;

        while (pEnumerator) {
            hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
            if (0 == uReturn) break;

            auto objCleanup = make_cleaner([pclsObj]() { pclsObj->Release(); });

            VARIANT vtProp;
            VariantInit(&vtProp);
            auto varCleanup = make_cleaner([&vtProp]() { VariantClear(&vtProp); });

            hr = pclsObj->Get(L"Manufacturer", 0, &vtProp, 0, 0);
            if (SUCCEEDED(hr) && vtProp.vt == VT_BSTR) {
                std::wstring manufacturer(vtProp.bstrVal);
                std::transform(manufacturer.begin(), manufacturer.end(), manufacturer.begin(), safe_towlower);

                if (manufacturer.find(L"vmware") != std::wstring::npos) {
                    return VM::VMware;
                }

                if (manufacturer.find(L"oracle") != std::wstring::npos ||
                    manufacturer.find(L"innotek") != std::wstring::npos) {
                    return VM::VirtualBox;
                }

                if (manufacturer.find(L"parallels") != std::wstring::npos) {
                    return VM::Parallels;
                }

                if (manufacturer.find(L"microsoft corporation") != std::wstring::npos) {
                    VARIANT modelProp;
                    VariantInit(&modelProp);
                    auto modelVarCleanup = make_cleaner([&modelProp]() { VariantClear(&modelProp); });

                    hr = pclsObj->Get(L"Model", 0, &modelProp, 0, 0);
                    if (SUCCEEDED(hr) && modelProp.vt == VT_BSTR) {
                        std::wstring model(modelProp.bstrVal);
                        std::transform(model.begin(), model.end(), model.begin(), safe_towlower);

                        if (model.find(L"virtual machine") != std::wstring::npos) {
                            return VM::HyperV_VirtualPC;
                        }
                    }
                }
            }
        }
        } while (false);

    // ---------------------------------------------- Don't use other methods for now ----------------------------------------------
#if 0

    // Method 3: Guest services detection (medium-high priority - indicates currently running VM)
    do {
        SC_HANDLE scManager = OpenSCManagerA(nullptr, nullptr, SC_MANAGER_ENUMERATE_SERVICE);
        if (!scManager) break;

        auto cleanup = make_cleaner([scManager]() { CloseServiceHandle(scManager); });

        struct ServiceCheck {
            const char* serviceName;
            VM vmType;
        };

        // Comprehensive list of VM services ordered by reliability
        const ServiceCheck checks[] = {
            // VMware services (active)
            {"VMTools", VM::VMware},
            {"vmware-tools", VM::VMware},
            {"vmci", VM::VMware},
            {"VMwareHostOpen", VM::VMware},
            {"vmrawdsk", VM::VMware},

            // VirtualBox services (active)
            {"VBoxGuest", VM::VirtualBox},
            {"VBoxService", VM::VirtualBox},
            {"VBoxSF", VM::VirtualBox},

            // Parallels services (active)
            {"prl_fs", VM::Parallels},
            {"prl_tg", VM::Parallels},
            {"prl_eth", VM::Parallels},

            // Hyper-V services (active)
            {"vmicheartbeat", VM::HyperV_VirtualPC},
            {"vmicvss", VM::HyperV_VirtualPC},
            {"vmicshutdown", VM::HyperV_VirtualPC}
        };

        for (const auto& check : checks) {
            SC_HANDLE service = OpenServiceA(scManager, check.serviceName, SERVICE_QUERY_STATUS);
            if (service) {
                auto serviceCleanup = make_cleaner([service]() { CloseServiceHandle(service); });

                SERVICE_STATUS status;
                if (QueryServiceStatus(service, &status) && status.dwCurrentState == SERVICE_RUNNING) {
                    return check.vmType;
                }
            }
        }
    } while (false);

    // Method 4: Guest processes detection (medium priority - indicates guest tools activity)
    do {
        HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (hSnapshot == INVALID_HANDLE_VALUE) break;

        auto cleanup = make_cleaner([hSnapshot]() { CloseHandle(hSnapshot); });

        PROCESSENTRY32 pe;
        pe.dwSize = sizeof(PROCESSENTRY32);

        if (!Process32First(hSnapshot, &pe)) break;

        do {
            std::string processName = pe.szExeFile;
            std::transform(processName.begin(), processName.end(), processName.begin(), safe_tolower);

            // VMware guest tools processes (running inside VM)
            if (processName == "vmtoolsd.exe") {
                return VM::VMware;
            }

            // VirtualBox processes (currently running)
            if (processName == "vboxservice.exe" || processName == "vboxtray.exe") {
                return VM::VirtualBox;
            }

            // Parallels processes (currently running)
            if (processName == "prl_cc.exe" || processName == "prl_tools.exe") {
                return VM::Parallels;
            }

        } while (Process32Next(hSnapshot, &pe));
    } while (false);

    // Method 5: Guest tools file detection (medium-low priority - guest additions/tools installed)
    {
        struct FileCheck {
            const char* path;
            VM vmType;
            bool isExecutable; // Prioritize executables that might be running
        };

        const FileCheck checks[] = {
            // Running executables (higher priority)
            {"C:\\Windows\\System32\\VBoxService.exe", VM::VirtualBox, true},
            {"C:\\Program Files\\VMware\\VMware Tools\\vmtoolsd.exe", VM::VMware, true},

            // Drivers (medium priority)
            {"C:\\Windows\\System32\\drivers\\VBoxGuest.sys", VM::VirtualBox, false},
            {"C:\\Windows\\System32\\drivers\\vmhgfs.sys", VM::VMware, false},
            {"C:\\Windows\\System32\\drivers\\vmmouse.sys", VM::VMware, false},
            {"C:\\Windows\\System32\\drivers\\vmxnet.sys", VM::VMware, false},
            {"C:\\Windows\\System32\\drivers\\vmci.sys", VM::VMware, false},
            {"C:\\Windows\\System32\\drivers\\VBoxMouse.sys", VM::VirtualBox, false},
            {"C:\\Windows\\System32\\drivers\\VBoxVideo.sys", VM::VirtualBox, false},
            {"C:\\Windows\\System32\\drivers\\prl_fs.sys", VM::Parallels, false},
            {"C:\\Windows\\System32\\drivers\\prl_tg.sys", VM::Parallels, false},
            {"C:\\Windows\\System32\\drivers\\vmgencounter.sys", VM::HyperV_VirtualPC, false},
            {"C:\\Windows\\System32\\drivers\\hyperkbd.sys", VM::HyperV_VirtualPC, false},

            // Installation directories (lowest priority - most likely historical)
            {"C:\\Program Files\\VMware\\VMware Tools\\", VM::VMware, false},
            {"C:\\Program Files\\Oracle\\VirtualBox Guest Additions\\", VM::VirtualBox, false}
        };

        for (const auto& check : checks) {
            DWORD attributes = GetFileAttributesA(check.path);
            if (attributes != INVALID_FILE_ATTRIBUTES) {
                return check.vmType;
            }
        }
    }

    // Method 6: Guest tools registry detection (low priority - guest additions/tools registry entries)
    {
        struct RegistryCheck {
            const char* path;
            VM vmType;
            bool isCurrentIndicator; // true for current state, false for potentially historical
        };

        const RegistryCheck checks[] = {
            // Current state indicators first (service entries)
            {"SYSTEM\\ControlSet001\\Services\\vmci", VM::VMware, true},
            {"SYSTEM\\ControlSet001\\Services\\VMTools", VM::VMware, true},
            {"SYSTEM\\ControlSet001\\Services\\vmrawdsk", VM::VMware, true},
            {"SYSTEM\\ControlSet001\\Services\\VBoxGuest", VM::VirtualBox, true},
            {"SYSTEM\\ControlSet001\\Services\\VBoxService", VM::VirtualBox, true},
            {"SYSTEM\\ControlSet001\\Services\\VBoxSF", VM::VirtualBox, true},
            {"SYSTEM\\ControlSet001\\Services\\prl_fs", VM::Parallels, true},
            {"SYSTEM\\ControlSet001\\Services\\prl_tg", VM::Parallels, true},
            {"SYSTEM\\ControlSet001\\Services\\prl_eth", VM::Parallels, true},
            {"SYSTEM\\ControlSet001\\Services\\vmicheartbeat", VM::HyperV_VirtualPC, true},
            {"SYSTEM\\ControlSet001\\Services\\vmicvss", VM::HyperV_VirtualPC, true},

            // Potentially historical indicators last (software installation entries)
            {"SOFTWARE\\VMware, Inc.\\VMware Tools", VM::VMware, false},
            {"SOFTWARE\\Oracle\\VirtualBox Guest Additions", VM::VirtualBox, false},
            {"SOFTWARE\\Microsoft\\Virtual Machine\\Guest\\Parameters", VM::HyperV_VirtualPC, false}
        };

        for (const auto& check : checks) {
            HKEY hKey;
            if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, check.path, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
                auto cleanup = make_cleaner([hKey]() { RegCloseKey(hKey); });
                return check.vmType;
            }
        }
    }

    // Method 7: MAC address-based detection (lowest priority - can be changed/spoofed)
    do {
        ULONG ulOutBufLen = 0;

        DWORD dwRetVal = GetAdaptersInfo(nullptr, &ulOutBufLen);
        if (dwRetVal != ERROR_BUFFER_OVERFLOW) break;

        IP_ADAPTER_INFO* pAdapterInfo = (IP_ADAPTER_INFO*)malloc(ulOutBufLen);
        if (!pAdapterInfo) break;

        auto cleanup = make_cleaner([pAdapterInfo]() { free(pAdapterInfo); });

        dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen);
        if (dwRetVal != NO_ERROR) break;

        IP_ADAPTER_INFO* pAdapter = pAdapterInfo;
        while (pAdapter) {
            if (pAdapter->AddressLength == 6) {
                // VMware MAC prefixes (prioritize most common/recent)
                if ((pAdapter->Address[0] == 0x00 && pAdapter->Address[1] == 0x50 && pAdapter->Address[2] == 0x56) ||
                    (pAdapter->Address[0] == 0x00 && pAdapter->Address[1] == 0x0C && pAdapter->Address[2] == 0x29) ||
                    (pAdapter->Address[0] == 0x00 && pAdapter->Address[1] == 0x1C && pAdapter->Address[2] == 0x14)) {
                    return VM::VMware;
                }

                // VirtualBox MAC prefix
                if (pAdapter->Address[0] == 0x08 && pAdapter->Address[1] == 0x00 && pAdapter->Address[2] == 0x27) {
                    return VM::VirtualBox;
                }

                // Parallels MAC prefix
                if (pAdapter->Address[0] == 0x00 && pAdapter->Address[1] == 0x1C && pAdapter->Address[2] == 0x42) {
                    return VM::Parallels;
                }
            }
            pAdapter = pAdapter->Next;
        }
        } while (false);

#endif // 0

#endif // UTILS_CPP_OS_WINDOWS

#ifdef UTILS_CPP_OS_LINUX

    // Method 1: DMI information detection (highest priority - hardware info)
    do {
        std::ifstream product_name("/sys/class/dmi/id/product_name");
        if (product_name) {
            std::string line;
            if (std::getline(product_name, line)) {
                std::transform(line.begin(), line.end(), line.begin(), safe_tolower);

                if (line.find("vmware") != std::string::npos) {
                    return VM::VMware;
                }
                if (line.find("virtualbox") != std::string::npos) {
                    return VM::VirtualBox;
                }
                if (line.find("kvm") != std::string::npos) {
                    return VM::KVM;
                }
                if (line.find("qemu") != std::string::npos) {
                    return VM::KVM;
                }
                if (line.find("parallels") != std::string::npos) {
                    return VM::Parallels;
                }
                if (line.find("xen") != std::string::npos) {
                    return VM::Xen;
                }
            }
        }

        std::ifstream sys_vendor("/sys/class/dmi/id/sys_vendor");
        if (sys_vendor) {
            std::string line;
            if (std::getline(sys_vendor, line)) {
                std::transform(line.begin(), line.end(), line.begin(), safe_tolower);

                if (line.find("vmware") != std::string::npos) {
                    return VM::VMware;
                }
                if (line.find("innotek") != std::string::npos ||
                    line.find("oracle") != std::string::npos) {
                    return VM::VirtualBox;
                }
                if (line.find("qemu") != std::string::npos) {
                    return VM::KVM;
                }
                if (line.find("parallels") != std::string::npos) {
                    return VM::Parallels;
                }
                if (line.find("xen") != std::string::npos) {
                    return VM::Xen;
                }
                if (line.find("microsoft corporation") != std::string::npos) {
                    return VM::HyperV_VirtualPC;
                }
            }
        }
    } while (false);

    // Method 2: Kernel module detection (high priority - currently loaded)
    do {
        std::ifstream modules("/proc/modules");
        if (!modules) break;

        std::string line;
        while (std::getline(modules, line)) {
            std::transform(line.begin(), line.end(), line.begin(), safe_tolower);

            if (line.find("vmw_") == 0 || line.find("vmware") != std::string::npos ||
                line.find("vmxnet") != std::string::npos || line.find("vmci") != std::string::npos ||
                line.find("vsock") != std::string::npos) {
                return VM::VMware;
            }
            if (line.find("vbox") != std::string::npos) {
                return VM::VirtualBox;
            }
            if (line.find("kvm") != std::string::npos || line.find("virtio") != std::string::npos) {
                return VM::KVM;
            }
            if (line.find("xen") != std::string::npos) {
                return VM::Xen;
            }
            if (line.find("hv_") == 0 || line.find("hyperv") != std::string::npos) {
                return VM::HyperV_VirtualPC;
            }
        }
    } while (false);

    // Method 3: Running process detection (medium-high priority)
    do {
        DIR* proc_dir = opendir("/proc");
        if (!proc_dir) break;

        struct dirent* entry;
        while ((entry = readdir(proc_dir)) != nullptr) {
            if (!safe_isdigit(entry->d_name[0])) continue;

            std::string comm_path = "/proc/" + std::string(entry->d_name) + "/comm";
            std::ifstream comm_file(comm_path);
            if (!comm_file) continue;

            std::string comm;
            if (std::getline(comm_file, comm)) {
                std::transform(comm.begin(), comm.end(), comm.begin(), safe_tolower);

                if (comm.find("vmware") != std::string::npos ||
                    comm == "vmtoolsd" || comm == "vmware-vmx") {
                    closedir(proc_dir);
                    return VM::VMware;
                }
                if (comm.find("vbox") != std::string::npos ||
                    comm == "vboxservice" || comm == "vboxclient") {
                    closedir(proc_dir);
                    return VM::VirtualBox;
                }
                if (comm == "qemu-ga" || comm.find("qemu") != std::string::npos) {
                    closedir(proc_dir);
                    return VM::KVM;
                }
                if (comm.find("parallels") != std::string::npos ||
                    comm == "prl_tools_service") {
                    closedir(proc_dir);
                    return VM::Parallels;
                }
            }
        }
        closedir(proc_dir);
    } while (false);

    // Method 4: File system detection (medium priority)
    {
        const char* vm_files[] = {
            "/usr/bin/vmware-toolbox-cmd",
            "/usr/bin/vmware-user",
            "/usr/bin/VBoxService",
            "/usr/bin/VBoxClient",
            "/sys/bus/pci/devices/0000:00:0f.0", // VMware SVGA
            "/proc/vz",  // OpenVZ/Virtuozzo
            "/proc/xen", // Xen
            nullptr
        };

        for (int i = 0; vm_files[i]; ++i) {
            struct stat st;
            if (stat(vm_files[i], &st) == 0) {
                const char* file = vm_files[i];
                if (strstr(file, "vmware") || strstr(file, "0000:00:0f.0")) {
                    return VM::VMware;
                }
                if (strstr(file, "VBox") || strstr(file, "vbox")) {
                    return VM::VirtualBox;
                }
                if (strstr(file, "xen")) {
                    return VM::Xen;
                }
            }
        }
    }

    // Method 5: Network interface MAC detection (lowest priority)
    do {
        std::ifstream net_dev("/proc/net/dev");
        if (!net_dev) break;

        std::string line;
        std::getline(net_dev, line); // Skip header
        std::getline(net_dev, line); // Skip header

        while (std::getline(net_dev, line)) {
            size_t colon_pos = line.find(':');
            if (colon_pos == std::string::npos) continue;

            std::string iface = line.substr(0, colon_pos);
            // Remove leading whitespace
            iface.erase(0, iface.find_first_not_of(" \t"));

            std::string address_path = "/sys/class/net/" + iface + "/address";
            std::ifstream addr_file(address_path);
            if (!addr_file) continue;

            std::string mac;
            if (std::getline(addr_file, mac) && mac.length() >= 8) {
                // Convert to lowercase for comparison
                std::transform(mac.begin(), mac.end(), mac.begin(), safe_tolower);

                // VMware MAC prefixes
                if (mac.substr(0, 8) == "00:50:56" ||
                    mac.substr(0, 8) == "00:0c:29" ||
                    mac.substr(0, 8) == "00:1c:14") {
                    return VM::VMware;
                }
                // VirtualBox MAC prefix
                if (mac.substr(0, 8) == "08:00:27") {
                    return VM::VirtualBox;
                }
                // Parallels MAC prefix
                if (mac.substr(0, 8) == "00:1c:42") {
                    return VM::Parallels;
                }
                // Xen MAC prefix
                if (mac.substr(0, 8) == "00:16:3e") {
                    return VM::Xen;
                }
            }
        }
    } while (false);

#endif // UTILS_CPP_OS_LINUX

#ifdef UTILS_CPP_OS_MACOS

    // Method 1: System hardware info via sysctl (highest priority)
    do {
        size_t size = 0;
        if (sysctlbyname("hw.model", nullptr, &size, nullptr, 0) != 0 || size == 0) break;

        std::string model(size, '\0');
        if (sysctlbyname("hw.model", &model[0], &size, nullptr, 0) != 0) break;

        // Remove null terminator if present
        if (!model.empty() && model.back() == '\0') {
            model.pop_back();
        }

        std::transform(model.begin(), model.end(), model.begin(), safe_tolower);

        if (model.find("vmware") != std::string::npos) {
            return VM::VMware;
        }
        if (model.find("virtualbox") != std::string::npos) {
            return VM::VirtualBox;
        }
        if (model.find("parallels") != std::string::npos) {
            return VM::Parallels;
        }
    } while (false);

    // Method 2: IOKit hardware registry inspection (high priority)
    do {
        #if defined(__MAC_OS_X_VERSION_MIN_REQUIRED) && __MAC_OS_X_VERSION_MIN_REQUIRED >= 120000
        io_service_t platform = IOServiceGetMatchingService(kIOMainPortDefault,
                                                           IOServiceMatching("IOPlatformExpertDevice"));
        #else
        io_service_t platform = IOServiceGetMatchingService(kIOMasterPortDefault,
                                                           IOServiceMatching("IOPlatformExpertDevice"));
        #endif

        if (platform == 0) break;

        auto cleanup = make_cleaner([platform]() { IOObjectRelease(platform); });

        CFPtr<CFTypeRef> manufacturerRef(
            IORegistryEntryCreateCFProperty(platform, CFSTR("manufacturer"),
                                          kCFAllocatorDefault, 0));

        if (manufacturerRef && CFGetTypeID(manufacturerRef.get()) == CFStringGetTypeID()) {
            CFStringRef manufacturer = static_cast<CFStringRef>(manufacturerRef.get());
            char buffer[256];
            if (CFStringGetCString(manufacturer, buffer, sizeof(buffer), kCFStringEncodingUTF8)) {
                std::string mfg = buffer;
                std::transform(mfg.begin(), mfg.end(), mfg.begin(), safe_tolower);

                if (mfg.find("vmware") != std::string::npos) {
                    return VM::VMware;
                }
                if (mfg.find("oracle") != std::string::npos ||
                    mfg.find("innotek") != std::string::npos) {
                    return VM::VirtualBox;
                }
                if (mfg.find("parallels") != std::string::npos) {
                    return VM::Parallels;
                }
            }
        }

        CFPtr<CFTypeRef> modelRef(
            IORegistryEntryCreateCFProperty(platform, CFSTR("model"),
                                          kCFAllocatorDefault, 0));

        if (modelRef && CFGetTypeID(modelRef.get()) == CFStringGetTypeID()) {
            CFStringRef model = static_cast<CFStringRef>(modelRef.get());
            char buffer[256];
            if (CFStringGetCString(model, buffer, sizeof(buffer), kCFStringEncodingUTF8)) {
                std::string modelStr = buffer;
                std::transform(modelStr.begin(), modelStr.end(), modelStr.begin(), safe_tolower);

                if (modelStr.find("vmware") != std::string::npos) {
                    return VM::VMware;
                }
                if (modelStr.find("virtualbox") != std::string::npos) {
                    return VM::VirtualBox;
                }
                if (modelStr.find("parallels") != std::string::npos) {
                    return VM::Parallels;
                }
            }
        }
    } while (false);

    // Method 3: Running process detection (medium priority)
    do {
        int process_count = proc_listallpids(nullptr, 0);
        if (process_count <= 0) break;

        std::vector<pid_t> pids(process_count);
        process_count = proc_listallpids(pids.data(), static_cast<int>(pids.size() * sizeof(pid_t)));
        if (process_count <= 0) break;

        pids.resize(process_count);

        for (pid_t pid : pids) {
            char name[PROC_PIDPATHINFO_MAXSIZE];
            if (proc_name(pid, name, sizeof(name)) <= 0) continue;

            std::string proc_name = name;
            std::transform(proc_name.begin(), proc_name.end(), proc_name.begin(), safe_tolower);

            if (proc_name.find("vmware") != std::string::npos) {
                return VM::VMware;
            }
            if (proc_name.find("virtualbox") != std::string::npos ||
                proc_name.find("vbox") != std::string::npos) {
                return VM::VirtualBox;
            }
            if (proc_name.find("parallels") != std::string::npos) {
                return VM::Parallels;
            }
        }
    } while (false);

    // Method 4: Network interface MAC detection (lowest priority)
    do {
        IfAddrsPtr ifap;
        if (getifaddrs(ifap.getRef()) != 0 || !ifap) break;

        for (struct ifaddrs* ifa = ifap.get(); ifa; ifa = ifa->ifa_next) {
            if (!ifa->ifa_addr || ifa->ifa_addr->sa_family != AF_LINK) continue;

            struct sockaddr_dl* sdl = reinterpret_cast<struct sockaddr_dl*>(ifa->ifa_addr);
            if (sdl->sdl_alen != 6) continue; // Only check 6-byte MAC addresses

            unsigned char* mac = reinterpret_cast<unsigned char*>(LLADDR(sdl));

            // VMware MAC prefixes
            if ((mac[0] == 0x00 && mac[1] == 0x50 && mac[2] == 0x56) ||
                (mac[0] == 0x00 && mac[1] == 0x0C && mac[2] == 0x29) ||
                (mac[0] == 0x00 && mac[1] == 0x1C && mac[2] == 0x14)) {
                return VM::VMware;
            }

            // VirtualBox MAC prefix
            if (mac[0] == 0x08 && mac[1] == 0x00 && mac[2] == 0x27) {
                return VM::VirtualBox;
            }

            // Parallels MAC prefix
            if (mac[0] == 0x00 && mac[1] == 0x1C && mac[2] == 0x42) {
                return VM::Parallels;
            }

            // Xen MAC prefix
            if (mac[0] == 0x00 && mac[1] == 0x16 && mac[2] == 0x3E) {
                return VM::Xen;
            }
        }
    } while (false);

#endif // UTILS_CPP_OS_MACOS

    return {};
}

} // namespace utils_cpp
