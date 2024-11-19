/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/utils-cpp
 * Contact:  ihor-drachuk-libs@pm.me  */

#include <utils-cpp/cpuid.h>

#include <cassert>

#ifdef UTILS_CPP_COMPILER_MSVC
#include <intrin.h>
#else
#ifndef UTILS_CPP_ARCH_ARM // Not ARM
#include <cpuid.h>
#endif // UTILS_CPP_ARCH_ARM
#endif // UTILS_CPP_COMPILER_MSVC

namespace utils_cpp {

namespace cpuid {

#ifdef UTILS_CPP_COMPILER_MSVC

bool get(Reg32 cpuInfo[RegCount], Reg32 functionId)
{
    __cpuid(reinterpret_cast<int*>(cpuInfo), *reinterpret_cast<int*>(&functionId));
    return true;
}

#else // Not MSVC

#ifndef UTILS_CPP_ARCH_ARM // Not ARM

bool get(Reg32 cpuInfo[RegCount], Reg32 functionId)
{
    return __get_cpuid(functionId, &cpuInfo[0], &cpuInfo[1], &cpuInfo[2], &cpuInfo[3]);
}

#else // ARM

bool get(Reg32 /*cpuInfo*/[RegCount], Reg32 /*functionId*/)
{
    return false;
}

#endif // UTILS_CPP_ARCH_ARM
#endif // UTILS_CPP_COMPILER_MSVC

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
    RawString cpuInfoStr = {};
    if (get(cpuInfoStr.data(), functionId)) {
        cpuInfoStr[cpuInfoStr.size() - 1] = '\0';
        return cpuInfoStr;
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
