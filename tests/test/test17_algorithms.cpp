/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/utils-qt
 * Contact:  ihor-drachuk-libs@pm.me  */

#include <gtest/gtest.h>
#include <utils-cpp/algorithms.h>

TEST(utils_cpp, test_algorithms_min)
{
    EXPECT_EQ(1, utils_cpp::min(1, 2));
    EXPECT_EQ(1, utils_cpp::min(3, 2, 1));
    EXPECT_EQ(1, utils_cpp::min(2, 1, 3));
    EXPECT_EQ(1, utils_cpp::min(3, 1, 4, 2));
    EXPECT_EQ(1, utils_cpp::min(2, 3, 1));
    EXPECT_EQ(1, utils_cpp::min(1, 3, 2));
}

TEST(utils_cpp, test_algorithms_max)
{
    EXPECT_EQ(2, utils_cpp::max(1, 2));
    EXPECT_EQ(3, utils_cpp::max(3, 2, 1));
    EXPECT_EQ(3, utils_cpp::max(2, 1, 3));
    EXPECT_EQ(4, utils_cpp::max(3, 1, 4, 2));
    EXPECT_EQ(3, utils_cpp::max(2, 3, 1));
    EXPECT_EQ(3, utils_cpp::max(1, 3, 2));
}

TEST(utils_cpp, test_algorithms_gcd)
{
    EXPECT_EQ(7, utils_cpp::gcd(7));
    EXPECT_EQ(7, utils_cpp::gcd(14, 21));
    EXPECT_EQ(7, utils_cpp::gcd(21, 14));
    EXPECT_EQ(7, utils_cpp::gcd(21, 14, 7));
    EXPECT_EQ(7, utils_cpp::gcd(21, 14, 7, 28));
    EXPECT_EQ(7, utils_cpp::gcd(21, 14, 7, 28, 35));
    EXPECT_EQ(10, utils_cpp::gcd(20, 30, 40));
    EXPECT_EQ(5, utils_cpp::gcd(20, 30, 45));
}

TEST(utils_cpp, test_algorithms_lcm)
{
    EXPECT_EQ(7, utils_cpp::lcm(7));
    EXPECT_EQ(42, utils_cpp::lcm(14, 21));
    EXPECT_EQ(42, utils_cpp::lcm(21, 14));
    EXPECT_EQ(42, utils_cpp::lcm(21, 14, 7));
    EXPECT_EQ(84, utils_cpp::lcm(21, 14, 7, 28));
    EXPECT_EQ(420, utils_cpp::lcm(21, 14, 7, 28, 35));
    EXPECT_EQ(120, utils_cpp::lcm(20, 30, 40));
    EXPECT_EQ(180, utils_cpp::lcm(20, 30, 45));
}
