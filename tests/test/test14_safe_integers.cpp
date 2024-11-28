/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/utils-qt
 * Contact:  ihor-drachuk-libs@pm.me  */

#include <gtest/gtest.h>
#include <utils-cpp/safe_integers.h>

TEST(utils_cpp, safe_integers)
{
    int32_t i32 = -1;
    uint32_t ui32 = i32;

    ASSERT_EQ(ui32, 0xFFFFFFFF);
    ASSERT_EQ(i32, ui32); // This is false, actually.
    ASSERT_NE(c(i32), ui32);
    ASSERT_LT(c(i32), ui32);
    ASSERT_GT(c(ui32), i32);

    ui32 = std::numeric_limits<unsigned int>::max();
    i32 = ui32;

    ASSERT_EQ(i32, 0xFFFFFFFF);
    ASSERT_EQ(i32, -1);
    ASSERT_EQ(ui32, i32); // This is false, actually.
    ASSERT_NE(c(i32), ui32);
    ASSERT_LT(c(i32), ui32);
    ASSERT_GT(c(ui32), i32);

    i32 = 0x7FFFFFFF;
    ui32 = i32;

    ASSERT_EQ(i32, 0x7FFFFFFF);
    ASSERT_EQ(ui32, 0x7FFFFFFF);
    ASSERT_EQ(i32, ui32);
    ASSERT_EQ(c(i32), ui32);
    ASSERT_EQ(c(ui32), i32);

    i32 = 0x80000000;
    ui32 = i32;

    ASSERT_EQ(i32, 0x80000000);
    ASSERT_EQ(ui32, 0x80000000);
    ASSERT_EQ(i32, ui32); // This is false, actually.
    ASSERT_NE(c(i32), ui32);
    ASSERT_NE(c(ui32), i32);

    i32 = 0;
    ui32 = i32;

    ASSERT_EQ(i32, 0);
    ASSERT_EQ(ui32, 0);
    ASSERT_EQ(i32, ui32);
    ASSERT_EQ(c(i32), ui32);
    ASSERT_EQ(c(ui32), i32);

    i32 = 1;

    ASSERT_GT(i32, ui32);
    ASSERT_GT(c(i32), ui32);
    ASSERT_LT(c(ui32), i32);

    int16_t i16 = 100;
    int8_t i8 = 100;
    ASSERT_EQ(c(i8), i16);
    ASSERT_EQ(c(i16), i8);

    uint16_t ui16 = std::numeric_limits<uint16_t>::max();
    //auto ui = s(ui16);
    //ASSERT_EQ(c(ui), ui16); // asserts!
    auto uiSafe = ss(ui16);
    ASSERT_TRUE((std::is_same_v<decltype(uiSafe), int32_t>));
    ASSERT_EQ(c(uiSafe), ui16);
}
