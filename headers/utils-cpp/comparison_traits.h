#pragma once
#include <type_traits>

namespace utils_cpp {

#define CREATE_IS_COMPARABLE(NAME, OP) \
template<typename A, typename B> \
struct is_comparable_##NAME { \
    template<typename T1, typename T2> \
    static auto test(int) -> decltype(std::declval<T1>() OP std::declval<T2>(), std::true_type()); \
 \
    template<typename, typename> \
    static auto test(...) -> std::false_type; \
 \
    static constexpr bool value = decltype(test<A, B>(0))::value; \
};

CREATE_IS_COMPARABLE(eq, ==);
CREATE_IS_COMPARABLE(ne, !=);
CREATE_IS_COMPARABLE(gt, >);
CREATE_IS_COMPARABLE(ge, >=);
CREATE_IS_COMPARABLE(lt, <);
CREATE_IS_COMPARABLE(le, <=);

} // namespace utils_cpp
