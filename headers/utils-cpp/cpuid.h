/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/utils-cpp
 * Contact:  ihor-drachuk-libs@pm.me  */

#include <array>
#include <cstdint>
#include <string>
#include <optional>

namespace utils_cpp {

namespace cpuid {

constexpr auto RegCount = 4;
using Reg32 = std::uint32_t;
using RawString = std::array<char, RegCount * sizeof(Reg32) + 1>;

enum Register
{
    eax,
    ebx,
    ecx,
    edx
};

#pragma pack(push, 1)
struct Registers
{
    Reg32 eax {};
    Reg32 ebx {};
    Reg32 ecx {};
    Reg32 edx {};

    bool getBit(Register reg, unsigned int bit) const;
};
#pragma pack(pop)

bool get(/*OUT*/ Reg32 cpuInfo[RegCount],
         /*IN*/  Reg32 functionId);

bool get(void* dst, Reg32 functionId);

std::optional<Registers> get(Reg32 functionId);

std::optional<RawString> getStringRaw(Reg32 functionId);

std::optional<std::string> getString(Reg32 functionId);

std::optional<bool> getBit(Reg32 functionId, Register reg, unsigned int bit);

} // namespace cpuid

} // namespace utils_cpp
