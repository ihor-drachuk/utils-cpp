/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/utils-cpp
 * Contact:  ihor-drachuk-libs@pm.me  */

#include <gtest/gtest.h>
#include <utils-cpp/variant_tools.h>
#include <string>

using namespace utils_cpp::variant_comparisons;

TEST(utils_cpp, variant_eq)
{
    std::variant<int, std::string> a;
    std::string b {"123"};
    ASSERT_TRUE(a != b);

    a = b;
    ASSERT_TRUE(a == b);

    a = 17;
    ASSERT_TRUE(a == 17);
    ASSERT_TRUE(a != b);
}

TEST(utils_cpp, variant_get_if)
{
    std::variant<int, std::string> a = "abc";
    ASSERT_EQ(*utils_cpp::get_if<std::string>(a), "abc");
    ASSERT_FALSE(utils_cpp::get_if<int>(a).has_value());
}
