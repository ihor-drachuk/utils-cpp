/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/utils-cpp
 * Contact:  ihor-drachuk-libs@pm.me  */

#pragma once
#include <optional>

namespace utils_cpp {

enum class ChassisTypeDetailed // Based on SMBIOS/DMI specification
{
    Desktop = 3,            // Desktop
    LowProfileDesktop = 4,  // Low Profile Desktop
    PizzaBox = 5,           // Pizza Box
    MiniTower = 6,          // Mini Tower
    Tower = 7,              // Tower
    Portable = 8,           // Portable
    Laptop = 9,             // Laptop
    Notebook = 10,          // Notebook
    HandHeld = 11,          // Hand Held
    DockingStation = 12,    // Docking Station
    AllInOne = 13,          // All in One
    SubNotebook = 14,       // Sub Notebook
    SpaceSaving = 15,       // Space-saving
    LunchBox = 16,          // Lunch Box
    MainServerChassis = 17, // Main Server Chassis
    ExpansionChassis = 18,  // Expansion Chassis
    SubChassis = 19,        // SubChassis
    BusExpansionChassis = 20, // Bus Expansion Chassis
    PeripheralChassis = 21, // Peripheral Chassis
    RaidChassis = 22,       // RAID Chassis
    RackMountChassis = 23,  // Rack Mount Chassis
    SealedCasePC = 24,      // Sealed-case PC
    MultiSystemChassis = 25, // Multi-system chassis
    CompactPCI = 26,        // Compact PCI
    AdvancedTCA = 27,       // Advanced TCA
    Blade = 28,             // Blade
    BladeEnclosure = 29,    // Blade Enclosure
    Tablet = 30,            // Tablet
    Convertible = 31,       // Convertible
    Detachable = 32,        // Detachable
    IoTGateway = 33,        // IoT Gateway
    EmbeddedPC = 34,        // Embedded PC
    MiniPC = 35,            // Mini PC
    StickPC = 36            // Stick PC
};

enum class ChassisTypeGeneralized
{
    Desktop,
    Laptop,
    Mobile,
    Server,
    Embedded
};

struct ChassisTypeMapping
{
    utils_cpp::ChassisTypeGeneralized typeGeneralized {};
    utils_cpp::ChassisTypeDetailed typeDetailed {};
    const char* name {""};
    const char* generalizedName {""};

    bool operator==(const ChassisTypeMapping& other) const;
};

namespace internal {

std::optional<ChassisTypeMapping> get_chassis_by_int(int chassisType);
std::optional<ChassisTypeMapping> get_chassis_by_name(const char* name);
std::optional<ChassisTypeMapping> get_chassis_by_mac_model(const char* macModel);

} // namespace internal

} // namespace utils_cpp
