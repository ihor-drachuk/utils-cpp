#pragma once
#include <cstdint>

inline constexpr std::uint32_t string_hash(const char* str, std::uint32_t hash = 2166136261UL) {
    return *str ? string_hash(str + 1, (hash ^ *str) * 16777619ULL) : hash;
}
