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

TEST(utils_cpp, safe_integers_cross_size_comparison)
{
    // Negative signed vs small unsigned of LARGER type
    // This is the key case: int8_t(-1) should be less than uint64_t(5)
    ASSERT_LT(c(int8_t(-1)),  uint64_t(5));
    ASSERT_GT(c(uint64_t(5)), int8_t(-1));

    ASSERT_LT(c(int8_t(-128)), uint64_t(0));
    ASSERT_GT(c(uint64_t(0)),  int8_t(-128));

    ASSERT_LT(c(int8_t(-1)),  uint32_t(0));
    ASSERT_GT(c(uint32_t(0)), int8_t(-1));

    ASSERT_LT(c(int16_t(-1)), uint64_t(5));
    ASSERT_GT(c(uint64_t(5)), int16_t(-1));

    ASSERT_LT(c(int32_t(-1)), uint64_t(5));
    ASSERT_GT(c(uint64_t(5)), int32_t(-1));

    // Negative signed vs unsigned of SMALLER type
    ASSERT_LT(c(int64_t(-1)), uint8_t(5));
    ASSERT_GT(c(uint8_t(5)),  int64_t(-1));

    ASSERT_LT(c(int32_t(-1)), uint8_t(0));
    ASSERT_GT(c(uint8_t(0)),  int32_t(-1));

    // Equal values across different sizes
    ASSERT_EQ(c(int8_t(5)),   uint64_t(5));
    ASSERT_EQ(c(uint64_t(5)), int8_t(5));
    ASSERT_EQ(c(int16_t(42)), uint32_t(42));

    // Positive signed vs large unsigned (value exceeds signed max)
    ASSERT_LT(c(int8_t(127)),  uint64_t(128));
    ASSERT_GT(c(uint64_t(128)), int8_t(127));

    // Zero cases across sizes
    ASSERT_EQ(c(int8_t(0)),   uint64_t(0));
    ASSERT_EQ(c(uint64_t(0)), int8_t(0));
    ASSERT_LT(c(int8_t(0)),   uint64_t(1));
    ASSERT_GT(c(uint64_t(1)), int8_t(0));
}

TEST(utils_cpp, safe_integers_u_s_ss)
{
    // u(): signed -> unsigned (same size)
    ASSERT_EQ(u(int8_t(0)), uint8_t(0));
    ASSERT_EQ(u(int8_t(127)), uint8_t(127));
    ASSERT_EQ(u(int32_t(42)), uint32_t(42));
    ASSERT_EQ(u(int64_t(1000)), uint64_t(1000));

    // u(): already unsigned -> passthrough
    ASSERT_EQ(u(uint16_t(500)), uint16_t(500));

    // s(): unsigned -> signed (same size)
    ASSERT_EQ(s(uint8_t(0)), int8_t(0));
    ASSERT_EQ(s(uint8_t(127)), int8_t(127));
    ASSERT_EQ(s(uint32_t(42)), int32_t(42));

    // s(): already signed -> passthrough
    ASSERT_EQ(s(int16_t(-1)), int16_t(-1));

    // ss(): unsigned -> signed of extended size (no data loss)
    ASSERT_EQ(ss(uint8_t(255)), int16_t(255));
    ASSERT_EQ(ss(uint16_t(65535)), int32_t(65535));
    ASSERT_EQ(ss(uint32_t(0xFFFFFFFF)), int64_t(0xFFFFFFFF));

    // ss(): already signed -> passthrough
    ASSERT_EQ(ss(int8_t(-1)), int8_t(-1));
}

TEST(utils_cpp, safe_integers_arbitrary_cast)
{
    // Same type
    ASSERT_EQ(u8(uint8_t(42)), uint8_t(42));
    ASSERT_EQ(s32(int32_t(-1)), int32_t(-1));

    // Narrowing that fits
    ASSERT_EQ(u8(uint64_t(200)), uint8_t(200));
    ASSERT_EQ(u8(uint32_t(0)), uint8_t(0));
    ASSERT_EQ(u8(uint16_t(255)), uint8_t(255));
    ASSERT_EQ(s8(int64_t(100)), int8_t(100));
    ASSERT_EQ(s8(int64_t(-128)), int8_t(-128));
    ASSERT_EQ(s8(int32_t(127)), int8_t(127));

    // Cross-signedness that fits (the bug case: unsigned -> signed narrow)
    ASSERT_EQ(s8(uint64_t(5)), int8_t(5));
    ASSERT_EQ(s8(uint64_t(127)), int8_t(127));
    ASSERT_EQ(s8(uint32_t(0)), int8_t(0));
    ASSERT_EQ(s16(uint64_t(32767)), int16_t(32767));

    // Signed -> unsigned narrow that fits
    ASSERT_EQ(u8(int64_t(5)), uint8_t(5));
    ASSERT_EQ(u8(int32_t(255)), uint8_t(255));
    ASSERT_EQ(u8(int16_t(0)), uint8_t(0));

    // Widening
    ASSERT_EQ(u64(uint8_t(42)), uint64_t(42));
    ASSERT_EQ(s64(int8_t(-1)), int64_t(-1));
    ASSERT_EQ(u32(uint8_t(255)), uint32_t(255));

    // toSizeT / z
    ASSERT_EQ(toSizeT(int32_t(42)), size_t(42));
    ASSERT_EQ(z(uint16_t(1000)), size_t(1000));
}

TEST(utils_cpp, safe_integers_underlying)
{
    enum class Color : uint8_t { Red = 1, Green = 2, Blue = 3 };
    enum class BigEnum : int32_t { Neg = -10, Zero = 0, Pos = 100 };

    ASSERT_EQ(underlying(Color::Red), uint8_t(1));
    ASSERT_EQ(underlying(Color::Blue), uint8_t(3));
    ASSERT_EQ(underlying(BigEnum::Neg), int32_t(-10));
    ASSERT_EQ(underlying(BigEnum::Zero), int32_t(0));
    ASSERT_EQ(underlying(BigEnum::Pos), int32_t(100));
}
