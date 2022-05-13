#include <gtest/gtest.h>
#include <utils-cpp/variant_eq_comparison.h>
#include <string>

TEST(UtilsCpp, variant_eq)
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
