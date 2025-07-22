/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/utils-cpp
 * Contact:  ihor-drachuk-libs@pm.me  */

#include <utils-cpp/vm_detector.h>

#include <utils-cpp/cpuid.h>
#include <utils-cpp/string_hash.h>

#ifdef UTILS_CPP_OS_WINDOWS
#include <windows.h>
#include <wbemidl.h>
#include <comdef.h>
#include <setupapi.h>
#include <algorithm>
#include <string>
#include <cctype>
#include <memory>

namespace {

// Generic RAII cleaner
template<typename F>
auto make_cleaner(F&& f) {
    auto deleter = [f = std::forward<F>(f)](void*){ f(); };
    return std::unique_ptr<void, decltype(deleter)>(reinterpret_cast<void*>(1), std::move(deleter));
}

} // anonymous namespace

#endif

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
                std::transform(desc.begin(), desc.end(), desc.begin(), ::tolower);

                if (desc.find("vmware") != std::string::npos) {
                    return VM::VMware;
                }
                if (desc.find("virtualbox") != std::string::npos || desc.find("vbox") != std::string::npos) {
                    return VM::VirtualBox;
                }
                if (desc.find("parallels") != std::string::npos) {
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
                std::transform(manufacturer.begin(), manufacturer.end(), manufacturer.begin(), ::towlower);

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
                        std::transform(model.begin(), model.end(), model.begin(), ::towlower);

                        if (model.find(L"virtual machine") != std::wstring::npos) {
                            return VM::HyperV_VirtualPC;
                        }
                    }
                }
            }

            pclsObj = nullptr;
        }
        } while (false);

    // ---------------------------------------------- Don't use other methods for now ----------------------------------------------
#if 0

    // Method 3: Active services detection (medium-high priority - indicates currently running VM)
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

    // Method 4: Running processes detection (medium priority - indicates current activity)
    do {
        HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (hSnapshot == INVALID_HANDLE_VALUE) break;

        auto cleanup = make_cleaner([hSnapshot]() { CloseHandle(hSnapshot); });

        PROCESSENTRY32 pe;
        pe.dwSize = sizeof(PROCESSENTRY32);

        if (!Process32First(hSnapshot, &pe)) break;

        do {
            std::string processName = pe.szExeFile;
            std::transform(processName.begin(), processName.end(), processName.begin(), ::tolower);

            // VMware processes (currently running)
            if (processName == "vmtoolsd.exe" || processName == "vmware-vmx.exe" ||
                processName == "vmware-authd.exe" || processName == "vmnetdhcp.exe") {
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

    // Method 5: File system-based detection (medium-low priority - potentially historical)
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

    // Method 6: Registry-based detection (low priority - might be historical)
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

    return {};
}

} // namespace utils_cpp
