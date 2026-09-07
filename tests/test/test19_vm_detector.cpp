/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/utils-cpp
 * Contact:  ihor-drachuk-libs@pm.me  */

#include <gtest/gtest.h>
#include <utils-cpp/cpuid.h>
#include <utils-cpp/vm_detector.h>

#include <atomic>
#include <chrono>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <initializer_list>
#include <optional>
#include <random>
#include <string>
#include <vector>

#include <Internal/vm_detector_platform.h>

using utils_cpp::FirmwareInfo;
using utils_cpp::VM;
using utils_cpp::VmEvidence;

namespace {

VmEvidence hypervisor(std::string vendor, bool rootPartition = false, bool nested = false)
{
    VmEvidence evidence;
    evidence.cpuidAvailable = true;
    evidence.hypervisorPresent = true;
    evidence.hypervisorVendor = std::move(vendor);
    evidence.hyperVRootPartition = rootPartition;
    evidence.hyperVNested = nested;
    return evidence;
}

VmEvidence firmware(std::string manufacturer, std::string product, bool cpuidAvailable = true)
{
    VmEvidence evidence;
    evidence.cpuidAvailable = cpuidAvailable;
    evidence.firmware.systemManufacturer = std::move(manufacturer);
    evidence.firmware.systemProduct = std::move(product);
    return evidence;
}

VmEvidence hypervisorType(std::string type)
{
    VmEvidence evidence;
    evidence.hypervisorType = std::move(type);
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

// Type 0 (vendor = string 1), type 1 (manufacturer = 1, product = 2, version = 3), type 2 (manufacturer = 1).
std::vector<std::byte> syntheticSmbiosTable()
{
    auto table = bytes({0x00, 0x18, 0x00, 0x00, 0x01, 0x02});   // type 0, length 24, handle 0
    table.resize(0x18, std::byte {0});
    appendString(table, "Phoenix Technologies LTD");
    appendString(table, "6.00");
    table.push_back(std::byte {0});

    const auto type1 = bytes({0x01, 0x08, 0x01, 0x00, 0x01, 0x02, 0x03, 0x04});   // type 1, length 8, handle 1
    table.insert(table.end(), type1.begin(), type1.end());
    appendString(table, "VMware, Inc.");
    appendString(table, "VMware Virtual Platform");
    appendString(table, "None");
    appendString(table, "VMware-42 1a");
    table.push_back(std::byte {0});

    const auto type2 = bytes({0x02, 0x08, 0x02, 0x00, 0x01, 0x02, 0x03, 0x04});   // type 2, length 8, handle 2
    table.insert(table.end(), type2.begin(), type2.end());
    appendString(table, "Intel Corporation");
    appendString(table, "440BX Desktop Reference Platform");
    appendString(table, "None");
    appendString(table, "None");
    table.push_back(std::byte {0});

    const auto end = bytes({0x7F, 0x04, 0x03, 0x00, 0x00, 0x00});   // end-of-table
    table.insert(table.end(), end.begin(), end.end());
    return table;
}

std::optional<VM> classifySmbios(const std::vector<std::byte>& table)
{
    VmEvidence evidence;
    evidence.firmware = utils_cpp::internal::parseSmbios(table.data(), table.size());
    return utils_cpp::classifyVm(evidence);
}

std::filesystem::path uniqueTempPath()
{
    static std::atomic<unsigned> counter {};
    const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
    const auto name = "utils-cpp-vm-" + std::to_string(stamp) + "-" + std::to_string(std::random_device {}()) + "-" +
                      std::to_string(counter++);
    return std::filesystem::temp_directory_path() / name;
}

struct TempDir
{
    std::filesystem::path path;

    TempDir()
        : path(uniqueTempPath())
    {
        std::filesystem::create_directories(path);
    }

    ~TempDir()
    {
        std::error_code ignored;
        std::filesystem::remove_all(path, ignored);
    }

    void write(const std::string& relativePath, const std::string& content) const
    {
        const auto file = path / std::filesystem::path(relativePath);
        std::filesystem::create_directories(file.parent_path());
        std::ofstream(file, std::ios::binary) << content;
    }

    std::string str() const { return path.string(); }
};

} // anonymous namespace

TEST(utils_cpp, ClassifyVm_FirmwareVmware_Vmware)
{
    EXPECT_EQ(utils_cpp::classifyVm(firmware("VMware, Inc.", "VMware Virtual Platform")), VM::VMware);
    EXPECT_EQ(utils_cpp::classifyVm(firmware("VMware, Inc.", "VMware20,1")), VM::VMware);
}

TEST(utils_cpp, ClassifyVm_FirmwareMacModelVmw_Vmware)
{
    EXPECT_EQ(utils_cpp::classifyVm(firmware("", "VMW7,1")), VM::VMware);
}

TEST(utils_cpp, ClassifyVm_FirmwareVirtualBox_VirtualBox)
{
    EXPECT_EQ(utils_cpp::classifyVm(firmware("innotek GmbH", "VirtualBox")), VM::VirtualBox);
}

TEST(utils_cpp, ClassifyVm_FirmwareOracleHardware_NotAGuest)
{
    EXPECT_FALSE(utils_cpp::classifyVm(firmware("Oracle Corporation", "SUN SERVER X4-2")).has_value());
}

TEST(utils_cpp, ClassifyVm_FirmwareParallels_Parallels)
{
    EXPECT_EQ(utils_cpp::classifyVm(firmware("Parallels Software International Inc.", "Parallels Virtual Platform")),
              VM::Parallels);
    EXPECT_EQ(utils_cpp::classifyVm(firmware("Parallels International GmbH", "Parallels ARM Virtual Machine")),
              VM::Parallels);
}

TEST(utils_cpp, ClassifyVm_FirmwareXen_Xen)
{
    EXPECT_EQ(utils_cpp::classifyVm(firmware("Xen", "HVM domU")), VM::Xen);
}

TEST(utils_cpp, ClassifyVm_FirmwareAppleVirtualization_Apple)
{
    EXPECT_EQ(utils_cpp::classifyVm(firmware("Apple Inc.", "Apple Virtualization Generic Platform")), VM::Apple);
}

TEST(utils_cpp, ClassifyVm_FirmwareVirtualMac_Apple)
{
    EXPECT_EQ(utils_cpp::classifyVm(firmware("Apple Inc.", "VirtualMac2,1")), VM::Apple);
    EXPECT_EQ(utils_cpp::classifyVm(firmware("", "VirtualMac2,1")), VM::Apple);
}

TEST(utils_cpp, ClassifyVm_FirmwareAppleHardware_NotAGuest)
{
    EXPECT_FALSE(utils_cpp::classifyVm(firmware("Apple Inc.", "MacBookPro18,3")).has_value());
}

TEST(utils_cpp, ClassifyVm_FirmwareBhyve_Bhyve)
{
    EXPECT_EQ(utils_cpp::classifyVm(firmware("BHYVE", "BHYVE")), VM::Bhyve);
}

TEST(utils_cpp, ClassifyVm_FirmwareMicrosoftVirtualMachine_HyperV)
{
    EXPECT_EQ(utils_cpp::classifyVm(firmware("Microsoft Corporation", "Virtual Machine")), VM::HyperV_VirtualPC);
}

TEST(utils_cpp, ClassifyVm_FirmwareMicrosoftHardware_NotAGuest)
{
    EXPECT_FALSE(utils_cpp::classifyVm(firmware("Microsoft Corporation", "Surface Laptop 4")).has_value());
}

TEST(utils_cpp, ClassifyVm_FirmwareHyperVProductVersion_HyperV)
{
    auto evidence = firmware("Microsoft Corporation", "");
    evidence.firmware.productVersion = "Hyper-V UEFI Release v4.1";
    EXPECT_EQ(utils_cpp::classifyVm(evidence), VM::HyperV_VirtualPC);
}

TEST(utils_cpp, ClassifyVm_FirmwareBoardAndBiosVendors_Scanned)
{
    VmEvidence board;
    board.firmware.boardManufacturer = "Parallels International GmbH";
    EXPECT_EQ(utils_cpp::classifyVm(board), VM::Parallels);

    VmEvidence bios;
    bios.firmware.biosVendor = "Xen";
    EXPECT_EQ(utils_cpp::classifyVm(bios), VM::Xen);
}

TEST(utils_cpp, ClassifyVm_FirmwareClouds_Kvm)
{
    EXPECT_EQ(utils_cpp::classifyVm(firmware("Amazon EC2", "t3.micro")), VM::KVM);
    EXPECT_EQ(utils_cpp::classifyVm(firmware("OpenStack Foundation", "OpenStack Nova")), VM::KVM);
    EXPECT_EQ(utils_cpp::classifyVm(firmware("Alibaba Cloud", "Alibaba Cloud ECS")), VM::KVM);
    EXPECT_EQ(utils_cpp::classifyVm(firmware("QEMU", "KVM Virtual Machine")), VM::KVM);
}

TEST(utils_cpp, ClassifyVm_GoogleComputeEngine_SignatureDecidesOnX86)
{
    auto guest = hypervisor("KVMKVMKVM");
    guest.firmware.systemManufacturer = "Google";
    guest.firmware.systemProduct = "Google Compute Engine";
    EXPECT_EQ(utils_cpp::classifyVm(guest), VM::KVM);

    EXPECT_FALSE(utils_cpp::classifyVm(firmware("Google", "Google Compute Engine")).has_value());
}

TEST(utils_cpp, ClassifyVm_GoogleComputeEngine_FirmwareDecidesWithoutCpuid)
{
    EXPECT_EQ(utils_cpp::classifyVm(firmware("Google", "Google Compute Engine", false)), VM::KVM);
}

TEST(utils_cpp, ClassifyVm_OpenStackWithVmwareSignature_Vmware)
{
    auto evidence = hypervisor("VMwareVMware");
    evidence.firmware.systemManufacturer = "OpenStack Foundation";
    evidence.firmware.systemProduct = "OpenStack Nova";
    EXPECT_EQ(utils_cpp::classifyVm(evidence), VM::VMware);
}

TEST(utils_cpp, ClassifyVm_UnknownTypeWithQemuFirmware_Qemu)
{
    auto evidence = hypervisorType("acme,hv");
    evidence.firmware.systemManufacturer = "QEMU";
    EXPECT_EQ(utils_cpp::classifyVm(evidence), VM::QEMU);
}

TEST(utils_cpp, ClassifyVm_FirmwareAmazonEc2Metal_NotAGuest)
{
    EXPECT_FALSE(utils_cpp::classifyVm(firmware("Amazon EC2", "c7g.metal")).has_value());
    EXPECT_FALSE(utils_cpp::classifyVm(firmware("Amazon EC2", "m5.metal-24xl")).has_value());
    EXPECT_EQ(utils_cpp::classifyVm(firmware("Amazon EC2", "x1.metallic")), VM::KVM);
}

TEST(utils_cpp, ClassifyVm_MetalOutsideAmazon_NotBareMetal)
{
    EXPECT_EQ(utils_cpp::classifyVm(firmware("VMware, Inc.", "VMware.metal")), VM::VMware);
}

TEST(utils_cpp, ClassifyVm_XenDom0_NotAGuest)
{
    auto evidence = hypervisor("XenVMMXenVMM");
    evidence.hypervisorType = "xen";
    evidence.xenDom0 = true;
    EXPECT_FALSE(utils_cpp::classifyVm(evidence).has_value());
}

TEST(utils_cpp, ClassifyVm_XenDomU_Xen)
{
    auto evidence = hypervisor("XenVMMXenVMM");
    evidence.hypervisorType = "xen";
    EXPECT_EQ(utils_cpp::classifyVm(evidence), VM::Xen);
}

TEST(utils_cpp, ClassifyVm_XenDom0WithOuterSignature_OuterVendor)
{
    auto vmware = hypervisor("VMwareVMware");
    vmware.hypervisorType = "xen";
    vmware.xenDom0 = true;
    EXPECT_EQ(utils_cpp::classifyVm(vmware), VM::VMware);

    auto hyperV = hypervisor("Microsoft Hv");
    hyperV.hypervisorType = "xen";
    hyperV.xenDom0 = true;
    EXPECT_EQ(utils_cpp::classifyVm(hyperV), VM::HyperV_VirtualPC);

    auto unknown = hypervisor("BhyveBhyve  ");
    unknown.hypervisorType = "xen";
    unknown.xenDom0 = true;
    EXPECT_EQ(utils_cpp::classifyVm(unknown), VM::Unknown);
}

TEST(utils_cpp, ClassifyVm_XenDom0WithVendorFirmware_FirmwareWins)
{
    auto evidence = hypervisor("XenVMMXenVMM");
    evidence.hypervisorType = "xen";
    evidence.xenDom0 = true;
    evidence.firmware.systemManufacturer = "VMware, Inc.";
    evidence.firmware.systemProduct = "VMware Virtual Platform";
    EXPECT_EQ(utils_cpp::classifyVm(evidence), VM::VMware);
}

TEST(utils_cpp, ClassifyVm_XenDom0WithQemuFirmware_NotAGuest)
{
    auto evidence = hypervisorType("xen");
    evidence.xenDom0 = true;
    evidence.firmware.systemManufacturer = "QEMU";
    EXPECT_FALSE(utils_cpp::classifyVm(evidence).has_value());
}

TEST(utils_cpp, ClassifyVm_FirmwareQemuOnly_Qemu)
{
    EXPECT_EQ(utils_cpp::classifyVm(firmware("QEMU", "Standard PC (Q35 + ICH9, 2009)")), VM::QEMU);
    EXPECT_EQ(utils_cpp::classifyVm(firmware("Bochs", "Bochs")), VM::QEMU);
}

TEST(utils_cpp, ClassifyVm_FirmwareQemuWithKvmSignature_Kvm)
{
    auto evidence = hypervisor("KVMKVMKVM");
    evidence.firmware.systemManufacturer = "QEMU";
    evidence.firmware.systemProduct = "Standard PC (Q35 + ICH9, 2009)";
    EXPECT_EQ(utils_cpp::classifyVm(evidence), VM::KVM);
}

TEST(utils_cpp, ClassifyVm_FirmwareQemuWithTcgSignature_Qemu)
{
    auto evidence = hypervisor("TCGTCGTCGTCG");
    evidence.firmware.systemManufacturer = "QEMU";
    EXPECT_EQ(utils_cpp::classifyVm(evidence), VM::QEMU);
}

TEST(utils_cpp, ClassifyVm_FirmwareQemuWithHyperVSignature_Qemu)
{
    auto evidence = hypervisor("Microsoft Hv");
    evidence.firmware.systemManufacturer = "QEMU";
    EXPECT_EQ(utils_cpp::classifyVm(evidence), VM::QEMU);
}

TEST(utils_cpp, ClassifyVm_FirmwareWinsOverHyperVRootPartition)
{
    auto evidence = hypervisor("Microsoft Hv", true);
    evidence.firmware.systemManufacturer = "VMware, Inc.";
    evidence.firmware.systemProduct = "VMware Virtual Platform";
    EXPECT_EQ(utils_cpp::classifyVm(evidence), VM::VMware);
}

TEST(utils_cpp, ClassifyVm_HyperVGuestWithNestedHyperV_HyperV)
{
    auto evidence = hypervisor("Microsoft Hv", true, true);
    evidence.firmware.systemManufacturer = "Microsoft Corporation";
    evidence.firmware.systemProduct = "Virtual Machine";
    EXPECT_EQ(utils_cpp::classifyVm(evidence), VM::HyperV_VirtualPC);
}

TEST(utils_cpp, ClassifyVm_HyperVRootPartition_NotAGuest)
{
    auto evidence = hypervisor("Microsoft Hv", true);
    evidence.firmware.systemManufacturer = "LENOVO";
    evidence.firmware.systemProduct = "20XW";
    EXPECT_FALSE(utils_cpp::classifyVm(evidence).has_value());
}

TEST(utils_cpp, ClassifyVm_HyperVNestedRootPartition_HyperV)
{
    EXPECT_EQ(utils_cpp::classifyVm(hypervisor("Microsoft Hv", true, true)), VM::HyperV_VirtualPC);
}

TEST(utils_cpp, ClassifyVm_MicrosoftHvGuest_HyperV)
{
    EXPECT_EQ(utils_cpp::classifyVm(hypervisor("Microsoft Hv", false)), VM::HyperV_VirtualPC);
}

TEST(utils_cpp, ClassifyVm_MicrosoftHvWithKvmNestedSignature_Kvm)
{
    auto evidence = hypervisor("Microsoft Hv", true);
    evidence.nestedHypervisorVendor = "KVMKVMKVM";
    EXPECT_EQ(utils_cpp::classifyVm(evidence), VM::KVM);
}

TEST(utils_cpp, ClassifyVm_HypervisorType_Table)
{
    EXPECT_EQ(utils_cpp::classifyVm(hypervisorType("xen")), VM::Xen);
    EXPECT_EQ(utils_cpp::classifyVm(hypervisorType("linux,kvm")), VM::KVM);
    EXPECT_EQ(utils_cpp::classifyVm(hypervisorType("vmware,vmware-vmi")), VM::VMware);
    EXPECT_EQ(utils_cpp::classifyVm(hypervisorType("acme,hv")), VM::Unknown);
}

TEST(utils_cpp, ClassifyVm_HypervisorTypeWinsOverSignature)
{
    auto evidence = hypervisor("Microsoft Hv");
    evidence.hypervisorType = "xen";
    EXPECT_EQ(utils_cpp::classifyVm(evidence), VM::Xen);
}

TEST(utils_cpp, ClassifyVm_CpuidVendors_Table)
{
    EXPECT_EQ(utils_cpp::classifyVm(hypervisor("VMwareVMware")), VM::VMware);
    EXPECT_EQ(utils_cpp::classifyVm(hypervisor("VBoxVBoxVBox")), VM::VirtualBox);
    EXPECT_EQ(utils_cpp::classifyVm(hypervisor("KVMKVMKVM")), VM::KVM);
    EXPECT_EQ(utils_cpp::classifyVm(hypervisor("Linux KVM Hv")), VM::KVM);
    EXPECT_EQ(utils_cpp::classifyVm(hypervisor("TCGTCGTCGTCG")), VM::QEMU);
    EXPECT_EQ(utils_cpp::classifyVm(hypervisor("XenVMMXenVMM")), VM::Xen);
    EXPECT_EQ(utils_cpp::classifyVm(hypervisor("bhyve bhyve")), VM::Bhyve);
    EXPECT_EQ(utils_cpp::classifyVm(hypervisor("Apple VZ")), VM::Apple);
    EXPECT_EQ(utils_cpp::classifyVm(hypervisor(" lrpepyh vr")), VM::Parallels);
    EXPECT_EQ(utils_cpp::classifyVm(hypervisor("prl hyperv")), VM::Parallels);
}

TEST(utils_cpp, ClassifyVm_UnknownVendor_Unknown)
{
    EXPECT_EQ(utils_cpp::classifyVm(hypervisor("BhyveBhyve  ")), VM::Unknown);
    EXPECT_EQ(utils_cpp::classifyVm(hypervisor("")), VM::Unknown);
}

TEST(utils_cpp, ClassifyVm_OsFlagOnly_Unknown)
{
    VmEvidence evidence;
    evidence.hypervisorPresent = true;
    evidence.firmware.systemManufacturer = "Apple Inc.";
    evidence.firmware.systemProduct = "Mac14,2";
    EXPECT_EQ(utils_cpp::classifyVm(evidence), VM::Unknown);
}

TEST(utils_cpp, ClassifyVm_NothingPresent_NotAGuest)
{
    EXPECT_FALSE(utils_cpp::classifyVm(VmEvidence {}).has_value());
    EXPECT_FALSE(utils_cpp::classifyVm(firmware("LENOVO", "20XW")).has_value());
}

TEST(utils_cpp, SmbiosTableAfterHeader_HeaderOrShorter_Empty)
{
    for (int size = 0; size <= 8; ++size)
        EXPECT_TRUE(utils_cpp::internal::smbiosTableAfterHeader(std::vector<std::byte>(size)).empty()) << size;
}

TEST(utils_cpp, SmbiosTableAfterHeader_PastHeader_Kept)
{
    std::vector<std::byte> raw(9);
    raw[8] = std::byte {0x7F};
    const auto table = utils_cpp::internal::smbiosTableAfterHeader(raw);
    ASSERT_EQ(table.size(), 1u);
    EXPECT_EQ(table[0], std::byte {0x7F});
}

TEST(utils_cpp, XenDom0FromFeatures_Bit11_Decides)
{
    EXPECT_TRUE(utils_cpp::internal::xenDom0FromFeatures("800"));
    EXPECT_TRUE(utils_cpp::internal::xenDom0FromFeatures("0x00000fff"));
    EXPECT_FALSE(utils_cpp::internal::xenDom0FromFeatures("7ff"));
    EXPECT_FALSE(utils_cpp::internal::xenDom0FromFeatures(""));
    EXPECT_FALSE(utils_cpp::internal::xenDom0FromFeatures("not hex"));
}

TEST(utils_cpp, ParseSmbios_SyntheticTable_ExtractsEveryField)
{
    const auto table = syntheticSmbiosTable();
    const auto info = utils_cpp::internal::parseSmbios(table.data(), table.size());
    EXPECT_EQ(info.biosVendor, "Phoenix Technologies LTD");
    EXPECT_EQ(info.systemManufacturer, "VMware, Inc.");
    EXPECT_EQ(info.systemProduct, "VMware Virtual Platform");
    EXPECT_EQ(info.productVersion, "None");
    EXPECT_EQ(info.boardManufacturer, "Intel Corporation");
    EXPECT_EQ(classifySmbios(table), VM::VMware);
}

TEST(utils_cpp, ParseSmbios_RecordWithoutStrings_KeepsWalking)
{
    auto table = bytes({0x00, 0x04, 0x00, 0x00});   // type 0, length 4, handle 0, no strings
    table.push_back(std::byte {0});
    table.push_back(std::byte {0});                 // a string-less record ends with two NUL bytes

    const auto type1 = bytes({0x01, 0x08, 0x01, 0x00, 0x01, 0x02, 0x00, 0x00});
    table.insert(table.end(), type1.begin(), type1.end());
    appendString(table, "VMware, Inc.");
    appendString(table, "VMware Virtual Platform");
    table.push_back(std::byte {0});

    const auto info = utils_cpp::internal::parseSmbios(table.data(), table.size());
    EXPECT_EQ(info.biosVendor, "");
    EXPECT_EQ(info.systemManufacturer, "VMware, Inc.");
    EXPECT_EQ(info.systemProduct, "VMware Virtual Platform");
}

TEST(utils_cpp, ParseSmbios_TwoRecordsWithoutStrings_KeepsWalking)
{
    auto table = bytes({0x00, 0x04, 0x00, 0x00, 0x00, 0x00});   // type 0, no strings
    const auto type2 = bytes({0x02, 0x04, 0x01, 0x00, 0x00, 0x00});   // type 2, no strings
    table.insert(table.end(), type2.begin(), type2.end());

    const auto type1 = bytes({0x01, 0x08, 0x02, 0x00, 0x01, 0x02, 0x00, 0x00});
    table.insert(table.end(), type1.begin(), type1.end());
    appendString(table, "innotek GmbH");
    appendString(table, "VirtualBox");
    table.push_back(std::byte {0});

    const auto info = utils_cpp::internal::parseSmbios(table.data(), table.size());
    EXPECT_EQ(info.systemManufacturer, "innotek GmbH");
    EXPECT_EQ(info.systemProduct, "VirtualBox");
    EXPECT_EQ(info.boardManufacturer, "");
}

TEST(utils_cpp, ParseSmbios_DuplicateRecord_FirstWins)
{
    auto table = bytes({0x01, 0x08, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00});
    appendString(table, "First");
    table.push_back(std::byte {0});
    const auto second = bytes({0x01, 0x08, 0x02, 0x00, 0x01, 0x00, 0x00, 0x00});
    table.insert(table.end(), second.begin(), second.end());
    appendString(table, "Second");
    table.push_back(std::byte {0});

    EXPECT_EQ(utils_cpp::internal::parseSmbios(table.data(), table.size()).systemManufacturer, "First");
}

TEST(utils_cpp, ParseSmbios_UnterminatedStringSet_Empty)
{
    auto table = bytes({0x00, 0x04, 0x00, 0x00});
    appendString(table, "BIOS Vendor");
    table.pop_back();   // drop the string's own terminator, so no record can follow

    const auto info = utils_cpp::internal::parseSmbios(table.data(), table.size());
    EXPECT_EQ(info.biosVendor, "");
    EXPECT_EQ(info.systemManufacturer, "");
}

TEST(utils_cpp, ParseSmbios_LengthAtBufferEnd_Empty)
{
    const auto table = bytes({0x01, 0x08, 0x01, 0x00, 0x01, 0x02, 0x00, 0x00});   // type 1, nothing after it

    const auto info = utils_cpp::internal::parseSmbios(table.data(), table.size());
    EXPECT_EQ(info.systemManufacturer, "");
    EXPECT_EQ(info.systemProduct, "");
}

TEST(utils_cpp, ParseSmbios_TruncatedTable_Empty)
{
    const auto full = syntheticSmbiosTable();
    const std::vector<std::byte> table(full.begin(), full.begin() + 20);
    const auto info = utils_cpp::internal::parseSmbios(table.data(), table.size());
    EXPECT_EQ(info.biosVendor, "");
    EXPECT_EQ(info.systemManufacturer, "");
}

TEST(utils_cpp, ParseSmbios_EmptyTable_Empty)
{
    const auto info = utils_cpp::internal::parseSmbios(nullptr, 0);
    EXPECT_EQ(info.systemManufacturer, "");
    EXPECT_EQ(info.systemProduct, "");
}

TEST(utils_cpp, ParseSmbios_StringIndexOutOfRange_Empty)
{
    auto table = bytes({0x01, 0x08, 0x01, 0x00, 0x05, 0x00, 0x00, 0x00});   // manufacturer index 5, product index 0
    appendString(table, "Only");
    table.push_back(std::byte {0});

    const auto info = utils_cpp::internal::parseSmbios(table.data(), table.size());
    EXPECT_EQ(info.systemManufacturer, "");
    EXPECT_EQ(info.systemProduct, "");
}

TEST(utils_cpp, ReadDmiFirmwareInfo_Directory_ReadsEveryFile)
{
    const TempDir dir;
    dir.write("bios_vendor", "SeaBIOS\n");
    dir.write("sys_vendor", "QEMU\n");
    dir.write("product_name", "Standard PC (Q35 + ICH9, 2009)\n");
    dir.write("product_version", "pc-q35-8.2\n");
    dir.write("board_vendor", "Oracle Corporation\n");

    const auto info = utils_cpp::internal::readDmiFirmwareInfo(dir.str());
    EXPECT_EQ(info.biosVendor, "SeaBIOS");
    EXPECT_EQ(info.systemManufacturer, "QEMU");
    EXPECT_EQ(info.systemProduct, "Standard PC (Q35 + ICH9, 2009)");
    EXPECT_EQ(info.productVersion, "pc-q35-8.2");
    EXPECT_EQ(info.boardManufacturer, "Oracle Corporation");
}

TEST(utils_cpp, ReadDmiFirmwareInfo_MissingFiles_Empty)
{
    const TempDir dir;
    dir.write("sys_vendor", "VMware, Inc.");

    const auto info = utils_cpp::internal::readDmiFirmwareInfo(dir.str());
    EXPECT_EQ(info.systemManufacturer, "VMware, Inc.");
    EXPECT_EQ(info.systemProduct, "");
    EXPECT_EQ(info.biosVendor, "");
    EXPECT_EQ(utils_cpp::internal::readDmiFirmwareInfo(dir.str() + "/absent").systemManufacturer, "");
}

TEST(utils_cpp, ReadTrimmedLine_TrailingWhitespaceAndNul_Dropped)
{
    const TempDir dir;
    dir.write("padded", std::string("Virtual Machine \t\0\r\n", 20));
    dir.write("multiline", "first\nsecond\n");
    dir.write("empty", "");

    EXPECT_EQ(utils_cpp::internal::readTrimmedLine(dir.str() + "/padded"), "Virtual Machine");
    EXPECT_EQ(utils_cpp::internal::readTrimmedLine(dir.str() + "/multiline"), "first");
    EXPECT_EQ(utils_cpp::internal::readTrimmedLine(dir.str() + "/empty"), "");
}

TEST(utils_cpp, ReadDeviceTreeHypervisor_Compatible_Read)
{
    const TempDir dir;
    dir.write("hypervisor/compatible", std::string("linux,kvm\0", 10));

    EXPECT_EQ(utils_cpp::internal::readDeviceTreeHypervisor(dir.str()), "linux,kvm");
    EXPECT_EQ(utils_cpp::internal::readDeviceTreeHypervisor(dir.str() + "/absent"), "");
}

TEST(utils_cpp, ReadDmiFirmwareInfo_XenDomU_ClassifiedAsXen)
{
    const TempDir dir;
    dir.write("sys_vendor", "Xen\n");
    dir.write("product_name", "HVM domU\n");

    VmEvidence evidence;
    evidence.firmware = utils_cpp::internal::readDmiFirmwareInfo(dir.str());
    EXPECT_EQ(utils_cpp::classifyVm(evidence), VM::Xen);
}

TEST(utils_cpp, ReadDeviceTreeHypervisor_LinuxKvm_ClassifiedAsKvm)
{
    const TempDir dir;
    dir.write("hypervisor/compatible", std::string("linux,kvm\0", 10));

    EXPECT_EQ(utils_cpp::classifyVm(hypervisorType(utils_cpp::internal::readDeviceTreeHypervisor(dir.str()))), VM::KVM);
}

TEST(utils_cpp, ReadHypervisorTypeIn_SysfsType_WinsOverProcXenAndDeviceTree)
{
    const TempDir sys;
    const TempDir proc;
    sys.write("hypervisor/type", "acme\n");
    proc.write("xen/capabilities", "");
    proc.write("device-tree/hypervisor/compatible", std::string("linux,kvm\0", 10));

    EXPECT_EQ(utils_cpp::internal::readHypervisorTypeIn(sys.str(), proc.str()), "acme");
}

TEST(utils_cpp, ReadHypervisorTypeIn_ProcXen_WinsOverDeviceTree)
{
    const TempDir sys;
    const TempDir proc;
    proc.write("xen/capabilities", "");
    proc.write("device-tree/hypervisor/compatible", std::string("linux,kvm\0", 10));

    EXPECT_EQ(utils_cpp::internal::readHypervisorTypeIn(sys.str(), proc.str()), "xen");
}

TEST(utils_cpp, ReadHypervisorTypeIn_DeviceTreeOnly_Compatible)
{
    const TempDir sys;
    const TempDir proc;
    proc.write("device-tree/hypervisor/compatible", std::string("linux,kvm\0", 10));

    EXPECT_EQ(utils_cpp::internal::readHypervisorTypeIn(sys.str(), proc.str()), "linux,kvm");
    EXPECT_EQ(utils_cpp::internal::readHypervisorTypeIn(sys.str(), proc.str() + "/absent"), "");
}

TEST(utils_cpp, ReadXenDom0In_Features_WinOverCapabilities)
{
    const TempDir sys;
    const TempDir proc;
    sys.write("hypervisor/properties/features", "7ff\n");
    proc.write("xen/capabilities", "control_d\n");

    EXPECT_FALSE(utils_cpp::internal::readXenDom0In(sys.str(), proc.str()));
}

TEST(utils_cpp, ReadXenDom0In_NoFeatures_CapabilitiesDecide)
{
    const TempDir sys;
    const TempDir proc;
    proc.write("xen/capabilities", "control_d\n");

    EXPECT_TRUE(utils_cpp::internal::readXenDom0In(sys.str(), proc.str()));
    EXPECT_FALSE(utils_cpp::internal::readXenDom0In(sys.str(), proc.str() + "/absent"));
}

TEST(utils_cpp, Cpuid_HypervisorLeaf_ReadableWhereCpuidIs)
{
    if (!utils_cpp::cpuid::get(0))
        return;
    EXPECT_TRUE(utils_cpp::cpuid::get(0x40000000).has_value());
}

TEST(utils_cpp, DetectVm_RealMachine_AgreesWithClassify)
{
    const auto evidence = utils_cpp::collectVmEvidence();
    EXPECT_EQ(utils_cpp::detectVm(), utils_cpp::classifyVm(evidence));

    if (evidence.hypervisorPresent && utils_cpp::cpuid::get(0))
        EXPECT_FALSE(evidence.hypervisorVendor.empty());
}

TEST(utils_cpp, CollectVmEvidence_RealMachine_CpuidAndOsFlagMerged)
{
    const auto evidence = utils_cpp::collectVmEvidence();
    const bool cpuidPresent = utils_cpp::cpuid::getBit(1, utils_cpp::cpuid::ecx, 31).value_or(false);
    EXPECT_EQ(evidence.cpuidAvailable, utils_cpp::cpuid::get(0).has_value());
    EXPECT_EQ(evidence.hypervisorPresent, cpuidPresent || utils_cpp::internal::readOsHypervisorFlag());
}

TEST(utils_cpp, CollectVmEvidence_RealMachine_FirmwareNamed)
{
    const auto& firmware = utils_cpp::collectVmEvidence().firmware;
    const bool named = !firmware.systemManufacturer.empty() || !firmware.systemProduct.empty() ||
                       !firmware.biosVendor.empty() || !firmware.boardManufacturer.empty();

#ifdef UTILS_CPP_OS_LINUX
    if (!std::filesystem::exists("/sys/class/dmi/id"))
        return;
#endif // UTILS_CPP_OS_LINUX
    EXPECT_TRUE(named);
}
