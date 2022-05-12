#include <gtest/gtest.h>
#include <utils-cpp/tuple_utils.h>

namespace {

int inc(int a)
{
    return a + 1;
}

double add(double a, int b)
{
    return a + b;
}

} // namespace

TEST(UtilsCpp, TupleUtilsTest_duplicate_type)
{
    using Type = duplicate_type<int, 20>::type;
    Type a;
    std::get<0>(a) = 1;
    std::get<10>(a) = 1;
    std::get<19>(a) = 1;
    //std::get<20>(a) = 1; // build error
}

TEST(UtilsCpp, TupleUtilsTest_call_multiple)
{
    auto results = call_multiple(inc, 1, 2, 3, 4);
    ASSERT_EQ(results, std::make_tuple(2, 3, 4, 5));

    auto results2 = call_multiple_tuple(inc, std::make_tuple(1,2,3,4));
    ASSERT_EQ(results2, std::make_tuple(2, 3, 4, 5));

    auto results3 = call_multiple(add,
                                  std::make_tuple(1,1),
                                  std::make_tuple(1,2),
                                  std::make_tuple(1,3),
                                  std::make_tuple(2,4)
                                  );

    ASSERT_EQ(results3, std::make_tuple(2, 3, 4, 6));
}

TEST(UtilsCpp, TupleUtilsTest_integer_sequence_to_tuple)
{
    auto value = integer_sequence_to_tuple(std::make_integer_sequence<int, 5>());
    ASSERT_EQ(value, std::make_tuple(0, 1, 2, 3, 4));
}

TEST(UtilsCpp, TupleUtilsTest_contains_type)
{
    auto result1 = contains_type<int, float, double, int, std::string>::value;
    ASSERT_TRUE(result1);

    auto result2 = contains_type<int, float, double, std::string>::value;
    ASSERT_FALSE(result2);

    auto result3 = contains_type_tuple<int, std::tuple<float, double, int, std::string>>::value;
    ASSERT_TRUE(result3);

    auto result4 = contains_type_tuple<int, std::tuple<float, double, std::string>>::value;
    ASSERT_FALSE(result4);
}
