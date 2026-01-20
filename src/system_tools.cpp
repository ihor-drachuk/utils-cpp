/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/utils-cpp
 * Contact:  ihor-drachuk-libs@pm.me  */

#include <utils-cpp/system_tools.h>
#include "Internal/system_tools_common.h"

#if defined(UTILS_CPP_OS_WINDOWS)
#include <Windows.h>
#else
#include <unistd.h>
#endif

namespace utils_cpp {

std::optional<bool> has_admin_rights()
{
#if 0 // Alternative solution
    bool fRet = false;
    HANDLE hToken = nullptr;
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
        TOKEN_ELEVATION Elevation;
        DWORD cbSize = sizeof(TOKEN_ELEVATION);
        if (GetTokenInformation(hToken, TokenElevation, &Elevation, sizeof(Elevation), &cbSize)) {
            fRet = Elevation.TokenIsElevated;
        }
    }
    if (hToken) {
        CloseHandle(hToken);
    }
    return fRet;
#endif

#ifdef UTILS_CPP_OS_WINDOWS
    BOOL isAdmin = FALSE;
    PSID adminGroup = nullptr;

    // Allocate and initialize a SID of the Administrators group
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    if (!AllocateAndInitializeSid(&NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &adminGroup))
        return {};

    // Check if the token of the calling process is a member of the Administrators group
    if (!CheckTokenMembership(nullptr, adminGroup, &isAdmin)) {
        FreeSid(adminGroup);
        return {};
    }

    // Clean up
    FreeSid(adminGroup);

    return isAdmin == TRUE;

#else
    return getuid() == 0;
#endif // UTILS_CPP_OS_WINDOWS
}

std::optional<ChassisTypeMapping> get_chassis_type()
{
    return internal::get_chassis_type();
}

std::optional<std::string> get_device_name()
{
    return internal::get_device_name();
}

} // namespace utils_cpp
