#pragma once
#include <type_traits>

namespace CheckMethod { }

#define CREATE_CHECK_METHOD(functionName) \
namespace CheckMethod { \
    template <class T> \
    struct helper_##functionName \
    { \
        template <typename C> static std::true_type check(decltype(&C::functionName)); \
        template <typename> static std::false_type check(...); \
        static bool const value = std::is_same<decltype(check<T>(0)), std::true_type>::value; \
    }; \
    template<class T> struct functionName: std::integral_constant<bool, CheckMethod::helper_##functionName<T>::value> {}; \
    template<class T> static constexpr bool functionName##_v = functionName<T>::value; \
} // namespace CheckMethod

//    Example
//  -----------
//
//  CREATE_CHECK_METHOD(asTuple);
//  CREATE_CHECK_METHOD(toJson);
//  
//  ... std::enable_if_t<CheckMethod::toJson_v<T>, int> ...
//