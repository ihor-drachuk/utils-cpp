#include <gtest/gtest.h>
#include <utils-cpp/struct_ops2.h>

namespace {

char charArray1[] = {'S', 't', 'r', 0, '1'};
char charArray2[] = {'S', 't', 'r', 0, '2'};
const int* const intPtr1 = (const int*)charArray1;
const int* const intPtr2 = (const int*)charArray2;

enum class ComparisonTest
{
    EQ,
    NE,
    LT,
    LE,
    GT,
    GE
};

struct MyStruct
{
    int a;
    float b;
    char* c;
    const int* d;

    auto tie() const { return std::tie(a, b, c, d); }
};

int compare(const MyStruct&, const int* lhs, const int* rhs)
{
    if (lhs == rhs) return 0;
    return strcmp((const char*)lhs, (const char*)rhs);
}

STRUCT_COMPARISONS2(MyStruct);


class StructOps3TestSuite : public testing::TestWithParam<std::tuple<MyStruct, MyStruct, ComparisonTest, bool>>
{
};

void test(const MyStruct& s1, const MyStruct& s2, ComparisonTest comparison, bool result)
{
    // Ensure these pointers are different, and data different as well. But strcmp == 0.
    // Otherwise test isn't valid.
    ASSERT_NE(intPtr1, intPtr2);
    ASSERT_NE(memcmp(charArray1, charArray2, sizeof(charArray1)), 0);
    ASSERT_EQ(strcmp(charArray1, charArray2), 0);

    bool answer = false;

    switch (comparison) {
        case ComparisonTest::EQ: answer = (s1 == s2); break;
        case ComparisonTest::NE: answer = (s1 != s2); break;
        case ComparisonTest::LT: answer = (s1  < s2); break;
        case ComparisonTest::LE: answer = (s1 <= s2); break;
        case ComparisonTest::GT: answer = (s1  > s2); break;
        case ComparisonTest::GE: answer = (s1 >= s2); break;
    }

    ASSERT_EQ(answer, result);
}

TEST_P(StructOps3TestSuite, DataDrivenTest)
{
    test(std::get<0>(GetParam()),
         std::get<1>(GetParam()),
         std::get<2>(GetParam()),
         std::get<3>(GetParam()));
}

} // namespace

INSTANTIATE_TEST_SUITE_P(
    StructOps3TestGroup,
    StructOps3TestSuite,
    testing::Values(
            // Basic operations
            std::make_tuple(MyStruct{0, 0, nullptr, nullptr}, MyStruct{0, 0, nullptr, nullptr}, ComparisonTest::EQ, true),
            std::make_tuple(MyStruct{0, 0, nullptr, nullptr}, MyStruct{0, 0, nullptr, nullptr}, ComparisonTest::NE, false),
            std::make_tuple(MyStruct{0, 0, nullptr, nullptr}, MyStruct{0, 0, nullptr, nullptr}, ComparisonTest::LE, true),
            std::make_tuple(MyStruct{0, 0, nullptr, nullptr}, MyStruct{0, 0, nullptr, nullptr}, ComparisonTest::GE, true),
            std::make_tuple(MyStruct{0, 0, nullptr, nullptr}, MyStruct{0, 0, nullptr, nullptr}, ComparisonTest::LT, false),
            std::make_tuple(MyStruct{0, 0, nullptr, nullptr}, MyStruct{0, 0, nullptr, nullptr}, ComparisonTest::GT, false),

            std::make_tuple(MyStruct{1, 0, nullptr, nullptr}, MyStruct{0, 0, nullptr, nullptr}, ComparisonTest::NE, true),
            std::make_tuple(MyStruct{1, 0, nullptr, nullptr}, MyStruct{0, 0, nullptr, nullptr}, ComparisonTest::GE, true),
            std::make_tuple(MyStruct{1, 0, nullptr, nullptr}, MyStruct{0, 0, nullptr, nullptr}, ComparisonTest::GT, true),
            std::make_tuple(MyStruct{1, 0, nullptr, nullptr}, MyStruct{0, 0, nullptr, nullptr}, ComparisonTest::LE, false),
            std::make_tuple(MyStruct{1, 0, nullptr, nullptr}, MyStruct{0, 0, nullptr, nullptr}, ComparisonTest::LT, false),

            std::make_tuple(MyStruct{0, 0, nullptr, nullptr}, MyStruct{1, 0, nullptr, nullptr}, ComparisonTest::NE, true),
            std::make_tuple(MyStruct{0, 0, nullptr, nullptr}, MyStruct{1, 0, nullptr, nullptr}, ComparisonTest::LE, true),
            std::make_tuple(MyStruct{0, 0, nullptr, nullptr}, MyStruct{1, 0, nullptr, nullptr}, ComparisonTest::LT, true),
            std::make_tuple(MyStruct{0, 0, nullptr, nullptr}, MyStruct{1, 0, nullptr, nullptr}, ComparisonTest::GE, false),
            std::make_tuple(MyStruct{0, 0, nullptr, nullptr}, MyStruct{1, 0, nullptr, nullptr}, ComparisonTest::GT, false),

            // Deep comparison
            std::make_tuple(MyStruct{0, 0, (char*)1, nullptr}, MyStruct{0, 0, nullptr, nullptr}, ComparisonTest::GT, true),
            std::make_tuple(MyStruct{0, 0, (char*)1, nullptr}, MyStruct{0, 0, nullptr, nullptr}, ComparisonTest::EQ, false),
            std::make_tuple(MyStruct{0, 0, (char*)1, nullptr}, MyStruct{0, 0, nullptr, nullptr}, ComparisonTest::NE, true),

            std::make_tuple(MyStruct{0, 0, nullptr, nullptr}, MyStruct{0, 0, (char*)1, nullptr}, ComparisonTest::LT, true),
            std::make_tuple(MyStruct{0, 0, nullptr, nullptr}, MyStruct{0, 0, (char*)1, nullptr}, ComparisonTest::EQ, false),
            std::make_tuple(MyStruct{0, 0, nullptr, nullptr}, MyStruct{0, 0, (char*)1, nullptr}, ComparisonTest::NE, true),

            // Float comparison
            std::make_tuple(MyStruct{0, 0, nullptr, nullptr}, MyStruct{0, std::numeric_limits<float>::epsilon()/(float)1.1, nullptr, nullptr}, ComparisonTest::EQ, true),
            std::make_tuple(MyStruct{0, 0, nullptr, nullptr}, MyStruct{0, std::numeric_limits<float>::epsilon()*2, nullptr, nullptr},   ComparisonTest::EQ, false),
            std::make_tuple(MyStruct{0, 0, nullptr, nullptr}, MyStruct{0, std::numeric_limits<float>::epsilon()*2, nullptr, nullptr},   ComparisonTest::LT, true),

            // Custom comparer
            std::make_tuple(MyStruct{0, 0, nullptr, intPtr1}, MyStruct{0, 0, nullptr, intPtr2}, ComparisonTest::EQ, true),

            // Stub
            std::make_tuple(MyStruct{0, 0, nullptr, nullptr}, MyStruct{0, 0, nullptr, nullptr}, ComparisonTest::EQ, true)
    ));
