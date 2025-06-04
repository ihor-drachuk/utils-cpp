/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/utils-cpp
 * Contact:  ihor-drachuk-libs@pm.me  */

#include <utils-cpp/Internal/chassis_types.h>

#include <cstring>
#include <algorithm>
#include <string>

namespace {

constexpr utils_cpp::ChassisTypeMapping ChassisTypeMappings[] = {
    // Desktop types
    { utils_cpp::ChassisTypeGeneralized::Desktop, utils_cpp::ChassisTypeDetailed::Desktop, "Desktop", "Desktop" },
    { utils_cpp::ChassisTypeGeneralized::Desktop, utils_cpp::ChassisTypeDetailed::LowProfileDesktop, "Low Profile Desktop", "Desktop" },
    { utils_cpp::ChassisTypeGeneralized::Desktop, utils_cpp::ChassisTypeDetailed::PizzaBox, "Pizza Box", "Desktop" },
    { utils_cpp::ChassisTypeGeneralized::Desktop, utils_cpp::ChassisTypeDetailed::MiniTower, "Mini Tower", "Desktop" },
    { utils_cpp::ChassisTypeGeneralized::Desktop, utils_cpp::ChassisTypeDetailed::Tower, "Tower", "Desktop" },
    { utils_cpp::ChassisTypeGeneralized::Desktop, utils_cpp::ChassisTypeDetailed::AllInOne, "All in One", "Desktop" },
    { utils_cpp::ChassisTypeGeneralized::Desktop, utils_cpp::ChassisTypeDetailed::SpaceSaving, "Space-saving", "Desktop" },
    { utils_cpp::ChassisTypeGeneralized::Desktop, utils_cpp::ChassisTypeDetailed::LunchBox, "Lunch Box", "Desktop" },
    { utils_cpp::ChassisTypeGeneralized::Desktop, utils_cpp::ChassisTypeDetailed::SealedCasePC, "Sealed-case PC", "Desktop" },
    { utils_cpp::ChassisTypeGeneralized::Desktop, utils_cpp::ChassisTypeDetailed::MiniPC, "Mini PC", "Desktop" },
    { utils_cpp::ChassisTypeGeneralized::Desktop, utils_cpp::ChassisTypeDetailed::StickPC, "Stick PC", "Desktop" },

    // Laptop types
    { utils_cpp::ChassisTypeGeneralized::Laptop, utils_cpp::ChassisTypeDetailed::Portable, "Portable", "Laptop" },
    { utils_cpp::ChassisTypeGeneralized::Laptop, utils_cpp::ChassisTypeDetailed::Laptop, "Laptop", "Laptop" },
    { utils_cpp::ChassisTypeGeneralized::Laptop, utils_cpp::ChassisTypeDetailed::Notebook, "Notebook", "Laptop" },
    { utils_cpp::ChassisTypeGeneralized::Laptop, utils_cpp::ChassisTypeDetailed::SubNotebook, "Sub Notebook", "Laptop" },

    // Mobile device types
    { utils_cpp::ChassisTypeGeneralized::Mobile, utils_cpp::ChassisTypeDetailed::HandHeld, "Hand Held", "Mobile" },
    { utils_cpp::ChassisTypeGeneralized::Mobile, utils_cpp::ChassisTypeDetailed::Tablet, "Tablet", "Mobile" },
    { utils_cpp::ChassisTypeGeneralized::Mobile, utils_cpp::ChassisTypeDetailed::Convertible, "Convertible", "Mobile" },
    { utils_cpp::ChassisTypeGeneralized::Mobile, utils_cpp::ChassisTypeDetailed::Detachable, "Detachable", "Mobile" },

    // Server types
    { utils_cpp::ChassisTypeGeneralized::Server, utils_cpp::ChassisTypeDetailed::MainServerChassis, "Main Server Chassis", "Server" },
    { utils_cpp::ChassisTypeGeneralized::Server, utils_cpp::ChassisTypeDetailed::ExpansionChassis, "Expansion Chassis", "Server" },
    { utils_cpp::ChassisTypeGeneralized::Server, utils_cpp::ChassisTypeDetailed::SubChassis, "SubChassis", "Server" },
    { utils_cpp::ChassisTypeGeneralized::Server, utils_cpp::ChassisTypeDetailed::BusExpansionChassis, "Bus Expansion Chassis", "Server" },
    { utils_cpp::ChassisTypeGeneralized::Server, utils_cpp::ChassisTypeDetailed::PeripheralChassis, "Peripheral Chassis", "Server" },
    { utils_cpp::ChassisTypeGeneralized::Server, utils_cpp::ChassisTypeDetailed::RaidChassis, "RAID Chassis", "Server" },
    { utils_cpp::ChassisTypeGeneralized::Server, utils_cpp::ChassisTypeDetailed::RackMountChassis, "Rack Mount Chassis", "Server" },
    { utils_cpp::ChassisTypeGeneralized::Server, utils_cpp::ChassisTypeDetailed::MultiSystemChassis, "Multi-system chassis", "Server" },
    { utils_cpp::ChassisTypeGeneralized::Server, utils_cpp::ChassisTypeDetailed::CompactPCI, "Compact PCI", "Server" },
    { utils_cpp::ChassisTypeGeneralized::Server, utils_cpp::ChassisTypeDetailed::AdvancedTCA, "Advanced TCA", "Server" },
    { utils_cpp::ChassisTypeGeneralized::Server, utils_cpp::ChassisTypeDetailed::Blade, "Blade", "Server" },
    { utils_cpp::ChassisTypeGeneralized::Server, utils_cpp::ChassisTypeDetailed::BladeEnclosure, "Blade Enclosure", "Server" },

    // Special types - categorized as embedded systems
    { utils_cpp::ChassisTypeGeneralized::Embedded, utils_cpp::ChassisTypeDetailed::DockingStation, "Docking Station", "Embedded" },
    { utils_cpp::ChassisTypeGeneralized::Embedded, utils_cpp::ChassisTypeDetailed::IoTGateway, "IoT Gateway", "Embedded" },
    { utils_cpp::ChassisTypeGeneralized::Embedded, utils_cpp::ChassisTypeDetailed::EmbeddedPC, "Embedded PC", "Embedded" }
};

} // namespace

namespace utils_cpp {

bool ChassisTypeMapping::operator==(const ChassisTypeMapping& other) const
{
    if (this == &other)
        return true;

    return typeGeneralized == other.typeGeneralized &&
           typeDetailed == other.typeDetailed &&
           !!name == !!other.name &&
           !!generalizedName == !!other.generalizedName &&
           (name == nullptr || std::strcmp(name, other.name) == 0) &&
           (generalizedName == nullptr || std::strcmp(generalizedName, other.generalizedName) == 0);
}

namespace internal {

std::optional<ChassisTypeMapping> get_chassis_by_int(int chassisType)
{
    for (const auto& mapping : ChassisTypeMappings) {
        if (mapping.typeDetailed == static_cast<utils_cpp::ChassisTypeDetailed>(chassisType)) {
            return mapping;
        }
    }
    return {};
}

std::optional<ChassisTypeMapping> get_chassis_by_name(const char* name)
{
    for (const auto& mapping : ChassisTypeMappings) {
        if (strcmp(mapping.name, name) == 0) {
            return mapping;
        }
    }
    return {};
}

std::optional<ChassisTypeMapping> get_chassis_by_mac_model(const char* macModel)
{
    if (!macModel)
        return {};

    // Convert model string to lowercase for easier matching
    std::string lowerModel = macModel;
    std::transform(lowerModel.begin(), lowerModel.end(), lowerModel.begin(), ::tolower);

    // Mac hardware model identification patterns
    if (lowerModel.find("macbook") != std::string::npos) {
        if (lowerModel.find("air") != std::string::npos) {
            return get_chassis_by_int(static_cast<int>(utils_cpp::ChassisTypeDetailed::Notebook));
        } else {
            return get_chassis_by_int(static_cast<int>(utils_cpp::ChassisTypeDetailed::Laptop));
        }
    }

    if (lowerModel.find("imac") != std::string::npos) {
        return get_chassis_by_int(static_cast<int>(utils_cpp::ChassisTypeDetailed::AllInOne));
    }

    if (lowerModel.find("mac mini") != std::string::npos || lowerModel.find("macmini") != std::string::npos) {
        return get_chassis_by_int(static_cast<int>(utils_cpp::ChassisTypeDetailed::MiniPC));
    }

    if (lowerModel.find("mac pro") != std::string::npos || lowerModel.find("macpro") != std::string::npos) {
        return get_chassis_by_int(static_cast<int>(utils_cpp::ChassisTypeDetailed::Tower));
    }

    if (lowerModel.find("mac studio") != std::string::npos || lowerModel.find("macstudio") != std::string::npos) {
        return get_chassis_by_int(static_cast<int>(utils_cpp::ChassisTypeDetailed::Desktop));
    }

    // VirtualMac detection (for CI/virtualized environments)
    if (lowerModel.find("virtualmac") != std::string::npos) {
        return get_chassis_by_int(static_cast<int>(utils_cpp::ChassisTypeDetailed::Desktop));
    }

    // iPad detection (if running on iPad)
    if (lowerModel.find("ipad") != std::string::npos) {
        return get_chassis_by_int(static_cast<int>(utils_cpp::ChassisTypeDetailed::Tablet));
    }

    // Return empty for unknown Mac models (don't assume default type)
    return {};
}

} // namespace internal

} // namespace utils_cpp
