#pragma once
#include <tuple>

/*

    Usage example
=====================

#include <utils-cpp/struct_ops.h>
#include <utils-cpp/struct_ops3.h>

struct A {
    int a1, a2;

    auto tie() const { return std::tie(a1, a2); }
    STRUCT_COMPARISONS_MEMBER(A);
};

struct B {
    int b;

    auto tie() const { return std::tie(b); }
    STRUCT_COMPARISONS_MEMBER(B);
};

struct C {
    std::string c;

    auto tie() const { return std::tie(c); }
    STRUCT_COMPARISONS_MEMBER(C);
};

struct Combo : A, B, C
{
    int ownValue;

    auto tie() const { return std::tie(ownValue); }
    STRUCT_INH_COMPARISONS3_MEMBER(Combo, A, B, C);
};

*/

namespace struct_ops_internal {

struct Comparer
{
    template<typename T>
    static int compare(const T& lhs, const T& rhs) {
        return lhs < rhs ? -1 :
               lhs > rhs ?  1 :
                            0;
    }

    template<typename T>
    static bool compareEq(const T& lhs, const T& rhs) {
        return lhs == rhs;
    }
};

template<typename T>
int compareInherited(const T& lhs, const T& rhs) {
    return lhs.tie() < rhs.tie() ? -1 :
           lhs.tie() > rhs.tie() ?  1 :
                                    0;
}

template<typename Arg0, typename... Args, typename T>
int compareInherited(const T& lhs, const T& rhs) {
    int result = Comparer::compare(static_cast<const Arg0&>(lhs), static_cast<const Arg0&>(rhs));
    return result == 0 ? compareInherited<Args...>(lhs, rhs) : result;
}

template<typename T>
bool compareInheritedEq(const T& lhs, const T& rhs) {
    return lhs.tie() == rhs.tie();
}

template<typename Arg0, typename... Args, typename T>
bool compareInheritedEq(const T& lhs, const T& rhs) {
    bool result = Comparer::compareEq(static_cast<const Arg0&>(lhs), static_cast<const Arg0&>(rhs));
    return result ? compareInheritedEq<Args...>(lhs, rhs) : false;
}

} // namespace struct_ops_internal

#define STRUCT_OP3(STRUCT, OP, ...) \
    inline bool operator OP(const STRUCT& lhs, const STRUCT& rhs) \
    { \
        const auto result = struct_ops_internal::compareInherited<__VA_ARGS__>(lhs, rhs); \
        return (result OP 0); \
    }

#define STRUCT_OP3_EQ(STRUCT, OP, ...) \
    inline bool operator OP(const STRUCT& lhs, const STRUCT& rhs) \
    { \
        const auto result = struct_ops_internal::compareTuplesEq<__VA_ARGS__>(lhs, rhs); \
        return (result OP true); \
    }

#define STRUCT_MEMBER_OP3(STRUCT, OP, ...) \
    inline bool operator OP(const STRUCT& rhs) const \
    { \
        const auto result = struct_ops_internal::compareInherited<__VA_ARGS__>(*this, rhs); \
        return (result OP 0); \
    }

#define STRUCT_MEMBER_OP3_EQ(STRUCT, OP, ...) \
    inline bool operator OP(const STRUCT& rhs) const \
    { \
        const auto result = struct_ops_internal::compareTuplesEq<__VA_ARGS__>(*this, rhs); \
        return (result OP true); \
    }

#define STRUCT_INH_COMPARISONS3(STRUCT, ...) \
    STRUCT_OP3(STRUCT, ==, __VA_ARGS__) \
    STRUCT_OP3(STRUCT, !=, __VA_ARGS__) \
    STRUCT_OP3(STRUCT, < , __VA_ARGS__) \
    STRUCT_OP3(STRUCT, <=, __VA_ARGS__) \
    STRUCT_OP3(STRUCT, > , __VA_ARGS__) \
    STRUCT_OP3(STRUCT, >=, __VA_ARGS__)

#define STRUCT_INH_COMPARISONS3_ONLY_EQ(STRUCT, ...) \
    STRUCT_OP3_EQ(STRUCT, ==, __VA_ARGS__) \
    STRUCT_OP3_EQ(STRUCT, !=, __VA_ARGS__)

#define STRUCT_INH_COMPARISONS3_MEMBER(STRUCT, ...) \
    STRUCT_MEMBER_OP3(STRUCT, ==, __VA_ARGS__) \
    STRUCT_MEMBER_OP3(STRUCT, !=, __VA_ARGS__) \
    STRUCT_MEMBER_OP3(STRUCT, < , __VA_ARGS__) \
    STRUCT_MEMBER_OP3(STRUCT, <=, __VA_ARGS__) \
    STRUCT_MEMBER_OP3(STRUCT, > , __VA_ARGS__) \
    STRUCT_MEMBER_OP3(STRUCT, >=, __VA_ARGS__)

#define STRUCT_INH_COMPARISONS3_MEMBER_ONLY_EQ(STRUCT, ...) \
    STRUCT_MEMBER_OP3_EQ(STRUCT, ==, __VA_ARGS__) \
    STRUCT_MEMBER_OP3_EQ(STRUCT, !=, __VA_ARGS__)
