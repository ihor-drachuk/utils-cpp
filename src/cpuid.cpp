/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/utils-cpp
 * Contact:  ihor-drachuk-libs@pm.me  */

#include <utils-cpp/cpuid.h>

#include <cassert>

#if defined(_M_IX86) || defined(_M_X64) || defined(__i386__) || defined(__x86_64__)
#define UTILS_CPP_CPUID_X86 1
#endif // defined(_M_IX86) || defined(_M_X64) || defined(__i386__) || defined(__x86_64__)

#ifdef UTILS_CPP_CPUID_X86
#ifdef UTILS_CPP_COMPILER_MSVC
#include <intrin.h>
#else
#include <cpuid.h>
#endif // UTILS_CPP_COMPILER_MSVC
#endif // UTILS_CPP_CPUID_X86

namespace utils_cpp {

namespace cpuid {

#if defined(UTILS_CPP_CPUID_X86) && defined(UTILS_CPP_COMPILER_MSVC)

bool get(Reg32 cpuInfo[RegCount], Reg32 functionId)
{
    int registers[RegCount] {};
    __cpuid(registers, static_cast<int>(functionId));

    for (int i = 0; i < RegCount; ++i)
        cpuInfo[i] = static_cast<Reg32>(registers[i]);

    return true;
}

#elif defined(UTILS_CPP_CPUID_X86)

bool get(Reg32 cpuInfo[RegCount], Reg32 functionId)
{
    // __get_cpuid refuses leaves above the basic maximum, which every hypervisor leaf (0x4000xxxx) is.
    __cpuid(functionId, cpuInfo[0], cpuInfo[1], cpuInfo[2], cpuInfo[3]);
    return true;
}

#else // Not x86

bool get(Reg32 /*cpuInfo*/[RegCount], Reg32 /*functionId*/)
{
    return false;
}

#endif // UTILS_CPP_CPUID_X86

bool get(void* dst, Reg32 functionId)
{
    return get(static_cast<Reg32*>(dst), functionId);
}

std::optional<Registers> get(Reg32 functionId)
{
    Registers cpuInfo;
    if (get(&cpuInfo.eax, functionId)) {
        return cpuInfo;
    } else {
        return {};
    }
}

std::optional<RawString> getStringRaw(Reg32 functionId)
{
#pragma pack(push, 1)
    struct Result
    {
        Reg32 eax {};
        RawString str {};
    };
    static_assert(sizeof(Result) == sizeof(Registers) + 1);
#pragma pack(pop)

    Result result = {};
    if (get(&result, functionId)) {
        result.str[result.str.size() - 1] = '\0';
        return result.str;
    } else {
        return {};
    }
}

std::optional<std::string> getString(Reg32 functionId)
{
    if (auto optInfo = getStringRaw(functionId)) {
        return std::string(optInfo->data());
    } else {
        return {};
    }
}

std::optional<bool> getBit(Reg32 functionId, Register reg, unsigned bit)
{
    auto optRegisters = get(functionId);
    if (!optRegisters)
        return {};

    return optRegisters->getBit(reg, bit);
}

bool Registers::getBit(Register reg, unsigned bit) const
{
    assert(bit < 32 && "Invalid bit value passed!");

    switch (reg) {
        case Register::eax: return (eax & (1u << bit)) != 0;
        case Register::ebx: return (ebx & (1u << bit)) != 0;
        case Register::ecx: return (ecx & (1u << bit)) != 0;
        case Register::edx: return (edx & (1u << bit)) != 0;
    }

    assert(false && "Invalid Register value passed!");
    return false;
}

} // namespace cpuid

} // namespace utils_cpp
