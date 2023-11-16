/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/utils-cpp
 * Contact:  ihor-drachuk-libs@pm.me  */

#include <gtest/gtest.h>
#include <utils-cpp/tuple_utils.h>
#include <string>

namespace {

int inc(int a)
{
    return a + 1;
}

double add(double a, int b)
{
    return a + b;
}

void voidFunc1(int) { }
void voidFunc2(int, const std::string&) { }

} // namespace

TEST(utils_cpp, TupleUtilsTest_duplicate_type)
{
    using Type = duplicate_type<int, 20>::type;
    Type a;
    std::get<0>(a) = 1;
    std::get<10>(a) = 1;
    std::get<19>(a) = 1;
    //std::get<20>(a) = 1; // build error
}

TEST(utils_cpp, TupleUtilsTest_for_each)
{
    auto results = for_each(inc, 1, 2, 3, 4);
    ASSERT_EQ(results, std::make_tuple(2, 3, 4, 5));

    auto results2 = for_each_tuple(inc, std::make_tuple(1,2,3,4));
    ASSERT_EQ(results2, std::make_tuple(2, 3, 4, 5));

    auto results3 = for_each(add,
                             std::make_tuple(1,1),
                             std::make_tuple(1,2),
                             std::make_tuple(1,3),
                             std::make_tuple(2,4)
                             );

    ASSERT_EQ(results3, std::make_tuple(2, 3, 4, 6));

    for_each(voidFunc1, 1, 2, 3);
    for_each(voidFunc2,
             std::make_tuple(1, std::string("11")),
             std::make_tuple(2, std::string("22")));

    std::string toBeCaptured;
    auto lambdaFunc1 =     [toBeCaptured](int a)            { (void)toBeCaptured; return a+1; };
    auto lambdaFunc2 =     [toBeCaptured](int a, int b)     { (void)toBeCaptured; return a+b; };
    auto lambdaFuncVoid1 = [toBeCaptured](int)              { (void)toBeCaptured; };
    auto lambdaFuncVoid2 = [toBeCaptured](int, std::string) { (void)toBeCaptured; };
    for_each(lambdaFunc1, 1, 2, 3, 4);
    for_each(lambdaFunc2, std::make_tuple(1, 1), std::make_tuple(2, 2));
    for_each(lambdaFuncVoid1, 1, 2, 3, 4);
    for_each(lambdaFuncVoid2, std::make_tuple(1, std::string("11")), std::make_tuple(2, std::string("22")));
}

TEST(utils_cpp, TupleUtilsTest_for_each_tuple_pair)
{
    auto data1 = std::make_tuple(1, 2, 3, 4);
    auto data2 = std::make_tuple(10, 20, 30, 40);
    int i = 0;
    for_each_tuple_pair([&i](auto& x1, auto& x2, auto ci) {
        x2 += x1;
        x1++;

        ASSERT_EQ(i++, decltype(ci)::I);
    }, data1, data2);

    ASSERT_EQ(data1, std::make_tuple(2, 3, 4, 5));
    ASSERT_EQ(data2, std::make_tuple(11, 22, 33, 44));
}

TEST(utils_cpp, TupleUtilsTest_integer_sequence_to_tuple)
{
    auto value = integer_sequence_to_tuple(std::make_integer_sequence<int, 5>());
    ASSERT_EQ(value, std::make_tuple(0, 1, 2, 3, 4));
}

TEST(utils_cpp, TupleUtilsTest_contains_type)
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
