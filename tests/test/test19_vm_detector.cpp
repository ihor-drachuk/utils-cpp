/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/utils-cpp
 * Contact:  ihor-drachuk-libs@pm.me  */

#include <gtest/gtest.h>
#include <utils-cpp/vm_detector.h>

#include <initializer_list>
#include <optional>
#include <vector>

using utils_cpp::VM;
using utils_cpp::VmEvidence;

namespace {

VmEvidence hypervisor(std::string vendor, bool rootPartition = false)
{
    VmEvidence evidence;
    evidence.hypervisorPresent = true;
    evidence.hypervisorVendor = std::move(vendor);
    evidence.rootPartition = rootPartition;
    return evidence;
}

VmEvidence smbios(std::string manufacturer, std::string product)
{
    VmEvidence evidence;
    evidence.smbiosManufacturer = std::move(manufacturer);
    evidence.smbiosProduct = std::move(product);
    return evidence;
}

std::vector<std::byte> bytes(std::initializer_list<int> values)
{
    std::vector<std::byte> result;
    for (int value : values)
        result.push_back(static_cast<std::byte>(value));
    return result;
}

void appendString(std::vector<std::byte>& table, const char* text)
{
    for (const char* p = text; *p; ++p)
        table.push_back(static_cast<std::byte>(*p));
    table.push_back(std::byte {0});
}

// Type 0 (BIOS, no strings we care about) followed by type 1 with manufacturer = string 1, product = string 2.
std::vector<std::byte> syntheticSmbiosTable()
{
    auto table = bytes({0x00, 0x04, 0x00, 0x00});   // type 0, length 4, handle 0
    appendString(table, "BIOS Vendor");
    table.push_back(std::byte {0});                 // end of the string set

    const auto type1 = bytes({0x01, 0x08, 0x01, 0x00, 0x01, 0x02, 0x03, 0x04});   // type 1, length 8, handle 1
    table.insert(table.end(), type1.begin(), type1.end());
    appendString(table, "VMware, Inc.");
    appendString(table, "VMware Virtual Platform");
    appendString(table, "None");
    appendString(table, "VMware-42 1a");
    table.push_back(std::byte {0});

    const auto end = bytes({0x7F, 0x04, 0x02, 0x00, 0x00, 0x00});   // end-of-table
    table.insert(table.end(), end.begin(), end.end());
    return table;
}

std::optional<VM> parse(const std::vector<std::byte>& table)
{
    const auto system = utils_cpp::parseSmbiosType1(table.data(), table.size());
    return utils_cpp::classifyVm(smbios(system.manufacturer, system.product));
}

} // anonymous namespace

TEST(utils_cpp, ClassifyVm_SmbiosVmware_Vmware)
{
    EXPECT_EQ(utils_cpp::classifyVm(smbios("VMware, Inc.", "VMware Virtual Platform")), VM::VMware);
}

TEST(utils_cpp, ClassifyVm_SmbiosVirtualBox_Virtualbox)
{
    EXPECT_EQ(utils_cpp::classifyVm(smbios("innotek GmbH", "VirtualBox")), VM::VirtualBox);
}

TEST(utils_cpp, ClassifyVm_SmbiosParallels_Parallels)
{
    EXPECT_EQ(utils_cpp::classifyVm(smbios("Parallels Software International Inc.", "Parallels Virtual Platform")),
              VM::Parallels);
}

TEST(utils_cpp, ClassifyVm_SmbiosHyperV_HyperV)
{
    EXPECT_EQ(utils_cpp::classifyVm(smbios("Microsoft Corporation", "Virtual Machine")), VM::HyperV_VirtualPC);
}

TEST(utils_cpp, ClassifyVm_MicrosoftHvRootPartition_NotAGuest)
{
    auto evidence = hypervisor("Microsoft Hv", true);
    evidence.smbiosManufacturer = "LENOVO";
    evidence.smbiosProduct = "20XW";
    EXPECT_FALSE(utils_cpp::classifyVm(evidence).has_value());
}

TEST(utils_cpp, ClassifyVm_MicrosoftHvGuest_HyperV)
{
    EXPECT_EQ(utils_cpp::classifyVm(hypervisor("Microsoft Hv", false)), VM::HyperV_VirtualPC);
}

TEST(utils_cpp, ClassifyVm_CpuidVendors_Table)
{
    EXPECT_EQ(utils_cpp::classifyVm(hypervisor("VMwareVMware")), VM::VMware);
    EXPECT_EQ(utils_cpp::classifyVm(hypervisor("VBoxVBoxVBox")), VM::VirtualBox);
    EXPECT_EQ(utils_cpp::classifyVm(hypervisor("KVMKVMKVM")), VM::KVM);
    EXPECT_EQ(utils_cpp::classifyVm(hypervisor("XenVMMXenVMM")), VM::Xen);
    EXPECT_EQ(utils_cpp::classifyVm(hypervisor(" lrpepyh vr")), VM::Parallels);
    EXPECT_EQ(utils_cpp::classifyVm(hypervisor("prl hyperv  ")), VM::Parallels);
}

TEST(utils_cpp, ClassifyVm_UnknownVendor_Unknown)
{
    EXPECT_EQ(utils_cpp::classifyVm(hypervisor("BhyveBhyve  ")), VM::Unknown);
    EXPECT_EQ(utils_cpp::classifyVm(hypervisor("")), VM::Unknown);
}

TEST(utils_cpp, ClassifyVm_NoHypervisorBit_NotAGuest)
{
    EXPECT_FALSE(utils_cpp::classifyVm(VmEvidence {}).has_value());
    EXPECT_FALSE(utils_cpp::classifyVm(smbios("LENOVO", "20XW")).has_value());
}

TEST(utils_cpp, ClassifyVm_SmbiosWinsOverCpuid)
{
    auto evidence = hypervisor("Microsoft Hv", false);
    evidence.smbiosManufacturer = "VMware, Inc.";
    evidence.smbiosProduct = "VMware Virtual Platform";
    EXPECT_EQ(utils_cpp::classifyVm(evidence), VM::VMware);
}

TEST(utils_cpp, ParseSmbiosType1_SyntheticTable_ExtractsManufacturerAndProduct)
{
    const auto table = syntheticSmbiosTable();
    const auto system = utils_cpp::parseSmbiosType1(table.data(), table.size());
    EXPECT_EQ(system.manufacturer, "VMware, Inc.");
    EXPECT_EQ(system.product, "VMware Virtual Platform");
    EXPECT_EQ(parse(table), VM::VMware);
}

TEST(utils_cpp, ParseSmbiosType1_RecordWithoutStrings_KeepsWalking)
{
    auto table = bytes({0x00, 0x04, 0x00, 0x00});   // type 0, length 4, handle 0, no strings
    table.push_back(std::byte {0});
    table.push_back(std::byte {0});                 // a string-less record ends with two NUL bytes

    const auto type1 = bytes({0x01, 0x08, 0x01, 0x00, 0x01, 0x02, 0x00, 0x00});
    table.insert(table.end(), type1.begin(), type1.end());
    appendString(table, "VMware, Inc.");
    appendString(table, "VMware Virtual Platform");
    table.push_back(std::byte {0});

    const auto system = utils_cpp::parseSmbiosType1(table.data(), table.size());
    EXPECT_EQ(system.manufacturer, "VMware, Inc.");
    EXPECT_EQ(system.product, "VMware Virtual Platform");
}

TEST(utils_cpp, ParseSmbiosType1_TwoRecordsWithoutStrings_KeepsWalking)
{
    auto table = bytes({0x00, 0x04, 0x00, 0x00, 0x00, 0x00});   // type 0, no strings
    const auto type2 = bytes({0x02, 0x04, 0x01, 0x00, 0x00, 0x00});   // type 2, no strings
    table.insert(table.end(), type2.begin(), type2.end());

    const auto type1 = bytes({0x01, 0x08, 0x02, 0x00, 0x01, 0x02, 0x00, 0x00});
    table.insert(table.end(), type1.begin(), type1.end());
    appendString(table, "innotek GmbH");
    appendString(table, "VirtualBox");
    table.push_back(std::byte {0});

    const auto system = utils_cpp::parseSmbiosType1(table.data(), table.size());
    EXPECT_EQ(system.manufacturer, "innotek GmbH");
    EXPECT_EQ(system.product, "VirtualBox");
}

TEST(utils_cpp, ParseSmbiosType1_UnterminatedStringSet_Empty)
{
    auto table = bytes({0x00, 0x04, 0x00, 0x00});   // type 0, whose string set runs to the end of the buffer
    appendString(table, "BIOS Vendor");
    table.pop_back();   // drop the string's own terminator, so no record can follow

    const auto system = utils_cpp::parseSmbiosType1(table.data(), table.size());
    EXPECT_EQ(system.manufacturer, "");
    EXPECT_EQ(system.product, "");
}

TEST(utils_cpp, ParseSmbiosType1_LengthAtBufferEnd_Empty)
{
    const auto table = bytes({0x01, 0x08, 0x01, 0x00, 0x01, 0x02, 0x00, 0x00});   // type 1, nothing after it

    const auto system = utils_cpp::parseSmbiosType1(table.data(), table.size());
    EXPECT_EQ(system.manufacturer, "");
    EXPECT_EQ(system.product, "");
}

TEST(utils_cpp, ParseSmbiosType1_TruncatedTable_Empty)
{
    const auto full = syntheticSmbiosTable();
    const std::vector<std::byte> table(full.begin(), full.begin() + 20);
    const auto system = utils_cpp::parseSmbiosType1(table.data(), table.size());
    EXPECT_EQ(system.manufacturer, "");
    EXPECT_EQ(system.product, "");
}

TEST(utils_cpp, ParseSmbiosType1_EmptyTable_Empty)
{
    const auto system = utils_cpp::parseSmbiosType1(nullptr, 0);
    EXPECT_EQ(system.manufacturer, "");
    EXPECT_EQ(system.product, "");
}

TEST(utils_cpp, ParseSmbiosType1_StringIndexOutOfRange_Empty)
{
    auto table = bytes({0x01, 0x08, 0x01, 0x00, 0x05, 0x00, 0x00, 0x00});   // manufacturer index 5, product index 0
    appendString(table, "Only");
    table.push_back(std::byte {0});

    const auto system = utils_cpp::parseSmbiosType1(table.data(), table.size());
    EXPECT_EQ(system.manufacturer, "");
    EXPECT_EQ(system.product, "");
}

TEST(utils_cpp, DetectVm_RealMachine_AgreesWithItsParts)
{
    const auto evidence = utils_cpp::collectVmEvidence();
    EXPECT_EQ(utils_cpp::detectVm(), utils_cpp::classifyVm(evidence));

    if (utils_cpp::detectVmOnly())
        EXPECT_EQ(utils_cpp::detectVm(), utils_cpp::detectVmOnly());
}
