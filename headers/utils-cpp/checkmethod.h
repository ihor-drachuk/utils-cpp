#pragma once
#include <type_traits>

namespace CheckMethod { }

#define CREATE_CHECK_METHOD(FunctionName) \
namespace CheckMethod { \
    template <typename T> \
    struct helper_##FunctionName \
    { \
        template <typename C> static std::true_type check(decltype(&C::FunctionName)); \
        template <typename> static std::false_type check(...); \
        static bool const value = std::is_same<decltype(check<T>(nullptr)), std::true_type>::value; \
    }; \
    template<class T> struct FunctionName: std::integral_constant<bool, CheckMethod::helper_##FunctionName<T>::value> {}; \
    template<class T> static constexpr bool FunctionName##_v = FunctionName<T>::value; \
} // namespace CheckMethod

//    Example
//  -----------
//
//  CREATE_CHECK_METHOD(asTuple);
//  CREATE_CHECK_METHOD(toJson);
//  
//  ... std::enable_if_t<CheckMethod::toJson_v<T>, int> ...
//
