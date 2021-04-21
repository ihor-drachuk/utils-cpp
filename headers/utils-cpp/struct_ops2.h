#pragma once
#include "tuple_for_each.h"
#include <cstring>
#include <cmath>
#include <tuple>

namespace struct_ops_internal {

template<typename T>
typename std::enable_if<!std::is_floating_point<T>::value, int>::type
inline compare(T a, T b)
{
    return (a > b) ? 1 :
           (a < b) ? -1 :
                     0;
}

inline int compare(const char* a, const char* b)
{
    return strcmp(a, b);
}

template <typename T>
typename std::enable_if<std::is_floating_point<T>::value, int>::type
inline compare(T a, T b)
{
    if (std::fabs(a - b) <= std::numeric_limits<T>::epsilon()) {
        return 0;
    } else {
        return (a > b) ? 1 : -1;
    }
}

template<typename StructTuple>
inline int compareTuples(const StructTuple& lhs, const StructTuple& rhs)
{
    auto result = for_each_pair_par<int>(lhs, rhs, [](int, const auto& v1, const auto& v2, const std::optional<int>&) -> std::optional<int> {
        const auto result = compare(v1, v2);
        return (result == 0) ? std::nullopt : std::optional<int>(result);
    }, std::nullopt, true);

    return result.value_or(0);
}

} // namespace struct_ops_internal

#define TIED_OP2(STRUCT, OP) \
    inline bool operator OP(const STRUCT& lhs, const STRUCT& rhs) \
    { \
        const auto result = struct_ops_internal::compareTuples(lhs.tie(), rhs.tie()); \
        return (result OP 0); \
    }

#define TIED_COMPARISONS2(STRUCT) \
    TIED_OP2(STRUCT, ==) \
    TIED_OP2(STRUCT, !=) \
    TIED_OP2(STRUCT, <) \
    TIED_OP2(STRUCT, <=) \
    TIED_OP2(STRUCT, >) \
    TIED_OP2(STRUCT, >=)

//    Example
//  -----------
//
//  struct My_Struct
//  {
//      int a_;
//      double b_;
//      auto tie() const { return std::tie(a_, b_); }
//  };
//
//  TIED_COMPARISONS2(My_Struct)
