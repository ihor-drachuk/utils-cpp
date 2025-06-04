/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/utils-cpp
 * Contact:  ihor-drachuk-libs@pm.me  */

#include "system_tools_common.h"

#include <utils-cpp/Internal/chassis_types.h>

#ifdef UTILS_CPP_OS_WINDOWS

#include <windows.h>  // For GetComputerNameA() in GetDeviceName()
#include <comdef.h>   // For _variant_t and WMI interfaces
#include <Wbemidl.h>  // For WMI interfaces in GetDeviceType()
#include <wrl/client.h> // For Microsoft::WRL::ComPtr

using Microsoft::WRL::ComPtr;

namespace utils_cpp {

namespace {

// RAII wrapper for COM initialization
class ComInitializer
{
public:
    ComInitializer(const ComInitializer&) = delete;
    ComInitializer& operator=(const ComInitializer&) = delete;

    ComInitializer() {
        HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);
        if (FAILED(hr))
            return;

        m_initialized = true;

        hr = CoInitializeSecurity(
            NULL, -1, NULL, NULL,
            RPC_C_AUTHN_LEVEL_DEFAULT,
            RPC_C_IMP_LEVEL_IMPERSONATE,
            NULL, EOAC_NONE, NULL
            );

        if (FAILED(hr)) {
            CoUninitialize();
            m_initialized = false;
        }
    }

    ~ComInitializer() {
        if (m_initialized)
            CoUninitialize();
    }

    bool IsInitialized() const { return m_initialized; }

private:
    bool m_initialized {};
};

} // namespace

namespace internal {

// Function to get the device type via WMI Win32_SystemEnclosure
std::optional<ChassisTypeMapping> get_chassis_type()
{
    // Initialize COM with RAII
    ComInitializer com;
    if (!com.IsInitialized())
        return {};

    // Create WMI locator
    ComPtr<IWbemLocator> pLoc;
    HRESULT hres = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID*)pLoc.GetAddressOf());
    if (FAILED(hres))
        return {};

    // Connect to WMI
    ComPtr<IWbemServices> pSvc;
    hres = pLoc->ConnectServer(_bstr_t(L"ROOT\\CIMV2"), NULL, NULL, 0, NULL, 0, 0, pSvc.GetAddressOf());
    if (FAILED(hres))
        return {};

    // Set security levels on the proxy
    hres = CoSetProxyBlanket(pSvc.Get(),
                             RPC_C_AUTHN_WINNT,
                             RPC_C_AUTHZ_NONE,
                             NULL,
                             RPC_C_AUTHN_LEVEL_CALL,
                             RPC_C_IMP_LEVEL_IMPERSONATE,
                             NULL,
                             EOAC_NONE);
    if (FAILED(hres))
        return {};

    // Execute WMI query
    ComPtr<IEnumWbemClassObject> pEnumerator;
    hres = pSvc->ExecQuery(_bstr_t(L"WQL"),
                           _bstr_t(L"SELECT * FROM Win32_SystemEnclosure"),
                           WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
                           NULL,
                           pEnumerator.GetAddressOf());
    if (FAILED(hres) || !pEnumerator)
        return {};

    // Get the first result
    ComPtr<IWbemClassObject> pclsObj;
    ULONG uReturn = 0;
    hres = pEnumerator->Next(WBEM_INFINITE, 1, pclsObj.GetAddressOf(), &uReturn);
    if (FAILED(hres) || uReturn == 0 || !pclsObj)
        return {};

    // Get ChassisTypes property using _variant_t for RAII
    _variant_t vtProp;
    hres = pclsObj->Get(L"ChassisTypes", 0, &vtProp, 0, 0);
    if (FAILED(hres) || !(vtProp.vt & VT_ARRAY))
        return {};

    // Process the SAFEARRAY
    SAFEARRAY* pArray = [&vtProp]() -> SAFEARRAY* {
        if (vtProp.vt & VT_BYREF) {
            return vtProp.pparray ? *vtProp.pparray : nullptr;
        } else {
            return vtProp.parray;
        }
    }();

    if (!pArray)
        return {};

    LONG lLBound, lUBound;
    if (FAILED(SafeArrayGetLBound(pArray, 1, &lLBound)) || FAILED(SafeArrayGetUBound(pArray, 1, &lUBound)))
        return {};

    // Check each chassis type
    for (LONG i = lLBound; i <= lUBound; i++) {
        INT chassisType;
        if (SUCCEEDED(SafeArrayGetElement(pArray, &i, &chassisType))) {
            const auto optChassisType = internal::get_chassis_by_int(chassisType);
            if (optChassisType)
                return optChassisType;
        }
    }

    return {};
}

std::optional<std::string> get_device_name()
{
    wchar_t buffer[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD size = sizeof(buffer) / sizeof(wchar_t);

    if (!GetComputerNameW(buffer, &size))
        return {};

    const auto sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, buffer, (int)size, nullptr, 0, nullptr, nullptr);

    if (sizeNeeded <= 0)
        return {};

    std::string result(sizeNeeded, 0);

    const auto convertedSize = WideCharToMultiByte(CP_UTF8, 0, buffer, (int)size, &result[0], sizeNeeded, nullptr, nullptr);

    if (convertedSize <= 0)
        return {};

    return result;
}

} // namespace internal

} // namespace utils_cpp

#endif // UTILS_CPP_OS_WINDOWS
