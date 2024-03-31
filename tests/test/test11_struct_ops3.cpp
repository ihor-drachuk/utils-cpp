/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/utils-cpp
 * Contact:  ihor-drachuk-libs@pm.me  */

#include <gtest/gtest.h>
#include <utils-cpp/struct_ops.h>
#include <utils-cpp/struct_ops3.h>
#include <string>
#include <array>

namespace {

struct MyStruct1
{
    int a {};
    int b {};

    auto tie() const { return std::tie(a, b); }
    STRUCT_COMPARISONS_MEMBER(MyStruct1);
};

struct MyStruct2
{
    std::string c;

    auto tie() const { return std::tie(c); }
    STRUCT_COMPARISONS_MEMBER(MyStruct2);
};

struct MyStruct : MyStruct1, MyStruct2
{
    int d {};

    auto tie() const { return std::tie(d); }
    STRUCT_INH_COMPARISONS3_MEMBER(MyStruct, MyStruct1, MyStruct2);
};

struct TestData
{
    MyStruct a, b;
    std::array<bool, 6> expResults;

    std::array<bool, 6> getActualResults() const {
        return {
            a == b,
            a != b,
            a <  b,
            a <= b,
            a >  b,
            a >= b
        };
    }
};


class StructOps3TestSuite : public testing::TestWithParam<TestData>
{
};

TEST_P(StructOps3TestSuite, DataDrivenTest)
{
    auto data = GetParam();
    auto actualResult = data.getActualResults();
    ASSERT_EQ(actualResult, data.expResults);
}

} // namespace

// clang-format off
#ifdef UTILS_CPP_COMPILER_CLANG
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-braces"
#endif // UTILS_CPP_COMPILER_CLANG
INSTANTIATE_TEST_SUITE_P(
    StructOps3TestGroup,
    StructOps3TestSuite,
    testing::Values( //               ==     !=     <     <=     >     >=
            TestData{{}, {},        {true, false, false, true, false, true}},
            TestData{{1, 0, {}, 0},
                     {0, 0, {}, 0}, {false, true, false, false, true, true}},
            TestData{{0, 1, {}, 0},
                     {0, 0, {}, 0}, {false, true, false, false, true, true}},
            TestData{{0,0,{"a"},0},
                     {0,0,{},   0}, {false, true, false, false, true, true}},
            TestData{{0, 0, {}, 1},
                     {0, 0, {}, 0}, {false, true, false, false, true, true}},
            TestData{{0, 0, {}, 0},
                     {1, 0, {}, 0}, {false, true, true,  true, false, false}},
            TestData{{0, 0, {}, 0},
                     {0, 0, {}, 1}, {false, true, true,  true, false, false}}
    )
);
#ifdef UTILS_CPP_COMPILER_CLANG
#pragma clang diagnostic pop
#endif // UTILS_CPP_COMPILER_CLANG
// clang-format on
