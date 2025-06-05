/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/utils-cpp
 * Contact:  ihor-drachuk-libs@pm.me  */

#include <gtest/gtest.h>
#include <utils-cpp/system_tools.h>
#include <utils-cpp/Internal/chassis_types.h>
#include <string>
#include <optional>
#include <cctype>
#include <iostream>

using namespace utils_cpp;

TEST(utils_cpp, system_tools_GetChassisType)
{
    const auto optChassisType = get_chassis_type();

    if (optChassisType) {
        ASSERT_TRUE(optChassisType->name != nullptr);
        ASSERT_TRUE(optChassisType->generalizedName != nullptr);
        ASSERT_FALSE(std::string(optChassisType->name).empty());
        ASSERT_FALSE(std::string(optChassisType->generalizedName).empty());
        std::cout << "Chassis type: " << optChassisType->name << std::endl;
        std::cout << "Generalized type: " << optChassisType->generalizedName << std::endl;
    }
}

TEST(utils_cpp, system_tools_GetDeviceName)
{
    const auto optDeviceName = get_device_name();

    ASSERT_TRUE(optDeviceName);
    ASSERT_FALSE(optDeviceName->empty());
    std::cout << "Device name: " << optDeviceName->c_str() << std::endl;
}

TEST(utils_cpp, system_tools_HasAdminRights)
{
    has_admin_rights();
}

TEST(utils_cpp, system_tools_MultipleCallsConsistency)
{
    // Test that multiple calls return consistent results
    const auto chassisType1 = get_chassis_type();
    const auto deviceName1 = get_device_name();
    const auto adminRights1 = has_admin_rights();

    const auto adminRights2 = has_admin_rights();
    const auto deviceName2 = get_device_name();
    const auto chassisType2 = get_chassis_type();

    ASSERT_EQ(chassisType1, chassisType2);
    ASSERT_EQ(deviceName1, deviceName2);
    ASSERT_EQ(adminRights1, adminRights2);
}

TEST(utils_cpp, system_tools_ChassisTypeMapping_ValidStrings)
{
    const auto chassisType = get_chassis_type();

    if (chassisType.has_value()) {
        const auto& mapping = chassisType.value();

        // Verify string pointers are not null and strings are not empty
        ASSERT_NE(mapping.name, nullptr);
        ASSERT_NE(mapping.generalizedName, nullptr);

        const std::string name = mapping.name;
        const std::string generalizedName = mapping.generalizedName;

        ASSERT_FALSE(name.empty());
        ASSERT_FALSE(generalizedName.empty());

        // Verify that name contains valid characters (basic ASCII check)
        for (char c : name) {
            ASSERT_TRUE(std::isprint(c) || std::isspace(c));
        }

        for (char c : generalizedName) {
            ASSERT_TRUE(std::isprint(c) || std::isspace(c));
        }
    }
}

TEST(utils_cpp, system_tools_AdminRights_ReturnType)
{
    has_admin_rights();
}

TEST(utils_cpp, system_tools_DeviceName)
{
    const auto optDeviceName = get_device_name();
    ASSERT_TRUE(optDeviceName.has_value());

    ASSERT_FALSE(optDeviceName->empty());

    // Basic validation that the string doesn't contain null bytes (except terminator)
    ASSERT_EQ(optDeviceName->find('\0'), std::string::npos);

    // Verify the string has reasonable length (not excessively long)
    ASSERT_LE(optDeviceName->length(), 256u); // Most system hostnames are limited to much less
}
