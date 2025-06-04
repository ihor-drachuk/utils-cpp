/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/utils-cpp
 * Contact:  ihor-drachuk-libs@pm.me  */

#include "system_tools_common.h"

#include <utils-cpp/Internal/chassis_types.h>

#ifdef UTILS_CPP_OS_MACOS

#include <algorithm>
#include <unistd.h>
#include <limits.h>
#include <sys/sysctl.h>
#include <IOKit/IOKitLib.h>
#include <CoreFoundation/CoreFoundation.h>

namespace utils_cpp {

namespace {

std::optional<std::string> get_sysctl_string(const char* name)
{
    size_t size = 0;

    // Get the size first
    if (sysctlbyname(name, nullptr, &size, nullptr, 0) != 0)
        return {};

    if (size == 0)
        return {};

    // Allocate buffer and get the actual value
    std::string result(size, '\0');
    if (sysctlbyname(name, &result[0], &size, nullptr, 0) != 0)
        return {};

    // Remove null terminator if present
    if (!result.empty() && result.back() == '\0')
        result.pop_back();

    return result;
}

std::optional<std::string> get_model_from_IOKit()
{
    // Try to get model from IOKit as fallback
    // Use kIOMainPortDefault for macOS 12.0+ to avoid deprecation warnings
#if defined(__MAC_OS_X_VERSION_MIN_REQUIRED) && __MAC_OS_X_VERSION_MIN_REQUIRED >= 120000
    io_service_t platformExpert = IOServiceGetMatchingService(kIOMainPortDefault,
                                                              IOServiceMatching("IOPlatformExpertDevice"));
#else
    io_service_t platformExpert = IOServiceGetMatchingService(kIOMasterPortDefault,
                                                              IOServiceMatching("IOPlatformExpertDevice"));
#endif

    if (platformExpert == 0)
        return {};

    using CFTypePtr = std::unique_ptr<const void, decltype(&CFRelease)>;
    CFTypePtr modelProperty(IORegistryEntryCreateCFProperty(platformExpert, CFSTR("model"), kCFAllocatorDefault, 0), CFRelease);
    IOObjectRelease(platformExpert);

    if (!modelProperty)
        return {};

    CFTypeID propertyTypeID = CFGetTypeID(modelProperty.get());

    if (propertyTypeID == CFStringGetTypeID()) {
        // Property is a CFString
        CFStringRef modelRef = (CFStringRef)modelProperty.get();

        CFIndex length = CFStringGetLength(modelRef);
        CFIndex maxSize = CFStringGetMaximumSizeForEncoding(length, kCFStringEncodingUTF8);

        if (maxSize <= 0 || maxSize > 1024)
            return {};

        std::string buffer(maxSize, '\0');
        Boolean status = CFStringGetCString(modelRef, buffer.data(), maxSize, kCFStringEncodingUTF8);

        if (status) {
            return std::string(buffer.c_str()); // String in buffer potentially contains redundant space.
        }

    } else if (propertyTypeID == CFDataGetTypeID()) {
        // Property is CFData, convert to string
        CFDataRef modelData = (CFDataRef)modelProperty.get();

        CFIndex dataLength = CFDataGetLength(modelData);
        const UInt8* dataBytes = CFDataGetBytePtr(modelData);

        if (dataBytes && dataLength > 0) {
            return std::string(reinterpret_cast<const char*>(dataBytes), dataLength);
        }

    } else {
        // Property is neither CFString nor CFData, unsupported type.
    }

    return {};
}

} // namespace

namespace internal {

std::optional<ChassisTypeMapping> get_chassis_type()
{
    // First try to get hardware model from sysctl
    const auto optModel = get_sysctl_string("hw.model");
    if (optModel) {
        const auto optChassis = get_chassis_by_mac_model(optModel->c_str());
        if (optChassis)
            return optChassis;
    }

    // Fallback to IOKit approach
    const auto optModelIOKit = get_model_from_IOKit();
    if (optModelIOKit) {
        return get_chassis_by_mac_model(optModelIOKit->c_str());
    }

    return {};
}

std::optional<std::string> get_device_name()
{
#ifndef HOST_NAME_MAX
    constexpr size_t HOST_NAME_MAX = 255;
#endif
    char hostname[HOST_NAME_MAX + 1];

    if (gethostname(hostname, sizeof(hostname)) != 0)
        return {};

    // Ensure null termination
    hostname[HOST_NAME_MAX] = '\0';

    return std::string(hostname);
}

} // namespace internal

} // namespace utils_cpp

#endif // UTILS_CPP_OS_MACOS
