/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/utils-cpp
 * Contact:  ihor-drachuk-libs@pm.me  */

#include <gtest/gtest.h>
#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <list>
#include <utils-cpp/container_utils.h>
#include <utils-cpp/comparison_traits.h>
#include <utils-cpp/checkmethod.h>
#include <utils-cpp/function_traits.h>

CREATE_CHECK_METHOD(index)


namespace {

template<typename T>
auto toDbl(T x) { return static_cast<double>(x); }

} // namespace


TEST(utils_cpp, ContainerUtilsTest_ByCopy)
{
    std::vector<int> vector {1, 2, 3};
    std::list<int> list {1, 2, 3};

    std::set<int> set {1, 2, 3};
    std::unordered_set<int> unordered_set {1, 2, 3};

    std::map<int, std::string> map {{1, "one"}, {2, "two"}, {3, "three"}};
    std::unordered_map<int, std::string> unordered_map {{1, "one"}, {2, "two"}, {3, "three"}};

    {
        const std::optional<int> optValue = utils_cpp::find(vector, 2);
        ASSERT_EQ(optValue.value(), 2);
    }

    {
        const bool found = !!utils_cpp::find(vector, 2);
        ASSERT_TRUE(found);
    }

    ASSERT_FALSE((utils_cpp::is_comparable_eq<decltype(utils_cpp::find(vector, 2)), bool>::value));
    ASSERT_FALSE((utils_cpp::is_comparable_eq<decltype(utils_cpp::find(vector, 2)), int>::value));
    ASSERT_FALSE((std::is_assignable<bool, decltype(utils_cpp::find(vector, 2))>::value));
    ASSERT_FALSE((std::is_assignable<int, decltype(utils_cpp::find(vector, 2))>::value));
    ASSERT_TRUE((std::is_assignable<std::optional<int>, decltype(utils_cpp::find(vector, 2))>::value));

    ASSERT_FALSE((std::is_assignable_v<decltype(utils_cpp::find(vector, 2)), int>));

    if (utils_cpp::find(vector, 1)) ASSERT_TRUE(true);
    if (!utils_cpp::find(vector, 4)) ASSERT_TRUE(true);
    if (utils_cpp::find(vector, 4)) ASSERT_TRUE(false);

    ASSERT_TRUE(utils_cpp::find(vector, 1).has_value());
    ASSERT_TRUE(utils_cpp::find(vector, 2).has_value());
    ASSERT_TRUE(utils_cpp::find(vector, 3).has_value());
    ASSERT_FALSE(utils_cpp::find(vector, 4).has_value());

    ASSERT_TRUE(utils_cpp::find(vector, 1));
    ASSERT_TRUE(utils_cpp::find(vector, 2));
    ASSERT_TRUE(utils_cpp::find(vector, 3));
    ASSERT_FALSE(utils_cpp::find(vector, 4));

    ASSERT_EQ(utils_cpp::find(vector, 1).value(), 1);
    ASSERT_EQ(utils_cpp::find(vector, 2).value(), 2);
    ASSERT_EQ(utils_cpp::find(vector, 3).value(), 3);
    ASSERT_EQ(utils_cpp::find(vector, 1).value_or_assert(), 1);
    ASSERT_EQ(utils_cpp::find(vector, 2).value_or_assert(), 2);
    ASSERT_EQ(utils_cpp::find(vector, 3).value_or_assert(), 3);

    ASSERT_TRUE(utils_cpp::find(list, 1).has_value());
    ASSERT_TRUE(utils_cpp::find(list, 2).has_value());
    ASSERT_EQ(utils_cpp::find(list, 2).index(), 1);
    ASSERT_TRUE(utils_cpp::find(list, 3).has_value());
    ASSERT_FALSE(utils_cpp::find(list, 4).has_value());

    ASSERT_TRUE(utils_cpp::find_in_map(map, 1).has_value());
    ASSERT_TRUE(utils_cpp::find_in_map(map, 2).has_value());
    ASSERT_TRUE(utils_cpp::find_in_map(map, 3).has_value());
    ASSERT_FALSE(utils_cpp::find_in_map(map, 4).has_value());
    ASSERT_TRUE(utils_cpp::find_in_map(unordered_map, 1).has_value());
    ASSERT_FALSE(utils_cpp::find_in_map(unordered_map, 4).has_value());
}

TEST(utils_cpp, ContainerUtilsTest_value_or)
{
    ASSERT_EQ(utils_cpp::find       (std::vector<int>   {1, 2, 3, 4}, 1)                        .value_or(10), 1);
    ASSERT_EQ(utils_cpp::find       (std::list<int>     {1, 2, 3, 4}, 2)                        .value_or(10), 2);
    ASSERT_EQ(utils_cpp::find_in_map(std::map<int, int> {{1, 11}, {2, 22}, {3, 33}, {4, 44}}, 4).value_or(10), 44);
    ASSERT_EQ(utils_cpp::find       (std::vector<int>   {1, 2, 3, 4}, 5)                        .value_or(10), 10);
}

TEST(utils_cpp, ContainerUtilsTest_ByRef)
{
    std::vector<int> container {1, 2, 3, 4};
    auto value = utils_cpp::find_ref(container, 2);
    container[1] = 5;
    ASSERT_EQ(*value, 5);

    ASSERT_TRUE((std::is_assignable_v<decltype(*value), int>));
    ASSERT_TRUE((CheckMethod::index_v<decltype(value)>));

    *value = 2;
    ASSERT_EQ(container[1], 2);
}

TEST(utils_cpp, ContainerUtilsTest_ByCRef)
{
    std::vector<int> container {1, 2, 3, 4};
    auto valueA = utils_cpp::find_cref(container, 2);
    container[1] = 5;
    ASSERT_EQ(*valueA, 5);
    ASSERT_EQ(valueA.index(), 1);
    ASSERT_FALSE((std::is_assignable_v<decltype(*valueA), int>));
    ASSERT_TRUE((CheckMethod::index_v<decltype(valueA)>));

    auto valueB = utils_cpp::find_ref(std::as_const(container), 3);
    container[2] = 10;
    ASSERT_EQ(*valueB, 10);
    ASSERT_EQ(valueB.index(), 2);
    ASSERT_FALSE((std::is_assignable_v<decltype(*valueB), int>));
    ASSERT_TRUE((CheckMethod::index_v<decltype(valueB)>));
}

TEST(utils_cpp, ContainerUtilsTest_MapByRef)
{
    std::map<int, int> mapContainer {
        {1, 11},
        {2, 22},
        {3, 33},
        {4, 44}
    };

    auto value = utils_cpp::find_in_map_ref(mapContainer, 2);
    ASSERT_EQ(*value, 22);
    mapContainer[2] = 220;
    ASSERT_EQ(*value, 220);
    ASSERT_TRUE((std::is_assignable_v<decltype(*value), int>));
    ASSERT_FALSE((CheckMethod::index_v<decltype(value)>));
    value.value() = 2200;
    ASSERT_EQ(mapContainer[2], 2200);
}

TEST(utils_cpp, ContainerUtilsTest_MapByCRef)
{
    {
        std::map<int, int> mapContainer {
            {1, 11},
            {2, 22},
            {3, 33},
            {4, 44}
        };

        auto value = utils_cpp::find_in_map_ref(std::as_const(mapContainer), 2);
        ASSERT_EQ(*value, 22);
        mapContainer[2] = 220;
        ASSERT_EQ(*value, 220);
        ASSERT_FALSE((std::is_assignable_v<decltype(*value), int>));
        ASSERT_FALSE((CheckMethod::index_v<decltype(value)>));
    }

    {
        std::map<int, int> mapContainer {
            {1, 11},
            {2, 22},
            {3, 33},
            {4, 44}
        };

        auto value = utils_cpp::find_in_map_cref(mapContainer, 2);
        ASSERT_EQ(*value, 22);
        mapContainer[2] = 220;
        ASSERT_EQ(*value, 220);
        ASSERT_FALSE((std::is_assignable_v<decltype(*value), int>));
        ASSERT_FALSE((CheckMethod::index_v<decltype(value)>));
    }
}

TEST(utils_cpp, ContainerUtilsTest_set)
{
    std::set<int> container {1, 2, 3, 4};
    //auto r = utils_cpp::find_ref(container, 2); -- should fail, because std::set::iterator isn't modifiable

    ASSERT_EQ(utils_cpp::transform<std::set>               (std::vector{1,2,3}, [](const auto& x){ return x; }), (std::set{1,2,3}));
    ASSERT_EQ(utils_cpp::transform<std::multiset>          (std::vector{1,2,3}, [](const auto& x){ return x; }), (std::multiset{1,2,3}));
    ASSERT_EQ(utils_cpp::transform<std::unordered_set>     (std::vector{1,2,3}, [](const auto& x){ return x; }), (std::unordered_set{1,2,3}));
    ASSERT_EQ(utils_cpp::transform<std::unordered_multiset>(std::vector{1,2,3}, [](const auto& x){ return x; }), (std::unordered_multiset{1,2,3}));
}

TEST(utils_cpp, ContainerUtilsTest_operators)
{
    struct SomeStruct {
        int value {};
        bool operator==(const SomeStruct& rhs) const { return rhs.value == value; }

        void testMethod() {}
        void testMethodConst() const {}
    };

    std::vector<SomeStruct> container {SomeStruct()};

    auto res = utils_cpp::find_ref(container, SomeStruct());
    (*res).testMethod();
    res->testMethod();

    auto resConst = utils_cpp::find_ref(std::as_const(container), SomeStruct());
    (*resConst).testMethodConst();
    resConst->testMethodConst();
}

TEST(utils_cpp, ContainerUtilsTest_contains)
{
    ASSERT_TRUE(utils_cpp::contains(std::vector<int> {1,2,3,4}, 2));
    ASSERT_TRUE(utils_cpp::contains_set(std::set<int> {1,2,3,4}, 2));
    ASSERT_TRUE(utils_cpp::contains_map(std::map<int, int> {{1, 11}, {2, 22}, {3, 33}, {4, 44}}, 2));
    ASSERT_TRUE(utils_cpp::contains_if(std::vector<int> {1,2,3,4}, [](auto x){ return x == 2; }));

    ASSERT_FALSE(utils_cpp::contains(std::vector<int> {1,2,3,4}, 10));
    ASSERT_FALSE(utils_cpp::contains_set(std::set<int> {1,2,3,4}, 10));
    ASSERT_FALSE(utils_cpp::contains_map(std::map<int, int> {{1, 11}, {2, 22}, {3, 33}, {4, 44}}, 10));
    ASSERT_FALSE(utils_cpp::contains_if(std::vector<int> {1,2,3,4}, [](auto x){ return x == 10; }));
}

TEST(utils_cpp, ContainerUtilsTest_count)
{
    // Test with vector - single occurrence
    ASSERT_EQ(utils_cpp::count(std::vector<int> {1,2,3,4}, 2), 1);
    ASSERT_EQ(utils_cpp::count(std::vector<int> {1,2,3,4}, 1), 1);
    ASSERT_EQ(utils_cpp::count(std::vector<int> {1,2,3,4}, 4), 1);

    // Test with vector - multiple occurrences
    ASSERT_EQ(utils_cpp::count(std::vector<int> {1,2,2,3,2,4}, 2), 3);
    ASSERT_EQ(utils_cpp::count(std::vector<int> {5,5,5,5}, 5), 4);

    // Test with vector - no occurrences
    ASSERT_EQ(utils_cpp::count(std::vector<int> {1,2,3,4}, 10), 0);
    ASSERT_EQ(utils_cpp::count(std::vector<int> {1,2,3,4}, 0), 0);

    // Test with empty vector
    ASSERT_EQ(utils_cpp::count(std::vector<int> {}, 1), 0);

    // Test with list
    ASSERT_EQ(utils_cpp::count(std::list<int> {1,2,2,3}, 2), 2);
    ASSERT_EQ(utils_cpp::count(std::list<int> {1,2,3,4}, 5), 0);

    // Test with strings
    ASSERT_EQ(utils_cpp::count(std::vector<std::string> {"a", "b", "c", "b"}, std::string("b")), 2);
    ASSERT_EQ(utils_cpp::count(std::vector<std::string> {"a", "b", "c"}, std::string("d")), 0);
}

TEST(utils_cpp, ContainerUtilsTest_count_if)
{
    // Test with simple predicates
    ASSERT_EQ(utils_cpp::count_if(std::vector<int> {1,2,3,4}, [](auto x){ return x == 2; }), 1);
    ASSERT_EQ(utils_cpp::count_if(std::vector<int> {1,2,3,4}, [](auto x){ return x > 2; }), 2);
    ASSERT_EQ(utils_cpp::count_if(std::vector<int> {1,2,3,4}, [](auto x){ return x < 3; }), 2);

    // Test with even/odd
    ASSERT_EQ(utils_cpp::count_if(std::vector<int> {1,2,3,4,5,6}, [](auto x){ return x % 2 == 0; }), 3);
    ASSERT_EQ(utils_cpp::count_if(std::vector<int> {1,2,3,4,5,6}, [](auto x){ return x % 2 == 1; }), 3);

    // Test with no matches
    ASSERT_EQ(utils_cpp::count_if(std::vector<int> {1,2,3,4}, [](auto x){ return x > 10; }), 0);

    // Test with all matches
    ASSERT_EQ(utils_cpp::count_if(std::vector<int> {1,2,3,4}, [](auto){ return true; }), 4);

    // Test with empty container
    ASSERT_EQ(utils_cpp::count_if(std::vector<int> {}, [](auto x){ return x > 0; }), 0);

    // Test with list
    ASSERT_EQ(utils_cpp::count_if(std::list<int> {10,20,30,40}, [](auto x){ return x >= 20; }), 3);

    // Test with strings
    ASSERT_EQ(utils_cpp::count_if(
        std::vector<std::string> {"apple", "banana", "apricot", "cherry"},
        [](const auto& s){ return s[0] == 'a'; }), 2);
}

TEST(utils_cpp, ContainerUtilsTest_accumulate)
{
    // Test basic sum with default init (0)
    ASSERT_EQ(utils_cpp::accumulate(std::vector<int> {1,2,3,4}), 10);
    ASSERT_EQ(utils_cpp::accumulate(std::vector<int> {1,2,3,4}, 0), 10);

    // Test sum with non-zero init
    ASSERT_EQ(utils_cpp::accumulate(std::vector<int> {1,2,3,4}, 10), 20);

    // Test with empty container (default behavior - returns init)
    ASSERT_EQ(utils_cpp::accumulate(std::vector<int> {}), 0);
    ASSERT_EQ(utils_cpp::accumulate(std::vector<int> {}, 100), 100);

    // Test with empty container and custom defaultValue
    ASSERT_EQ(utils_cpp::accumulate(std::vector<int> {}, 100, std::plus<>(), {-1}), -1);
    ASSERT_EQ(utils_cpp::accumulate(std::vector<int> {}, 10, std::plus<>(), {999}), 999);

    // Test with non-empty container and defaultValue (defaultValue should be ignored)
    ASSERT_EQ(utils_cpp::accumulate(std::vector<int> {1,2,3}, 0, std::plus<>(), {999}), 6);
    ASSERT_EQ(utils_cpp::accumulate(std::vector<int> {1,2,3}, 10, std::plus<>(), {999}), 16);

    // Test with multiplication
    ASSERT_EQ(utils_cpp::accumulate(std::vector<int> {1,2,3,4}, 1, std::multiplies<>()), 24);
    ASSERT_EQ(utils_cpp::accumulate(std::vector<int> {2,3,4}, 10, std::multiplies<>()), 240);

    // Test with custom non-associative operation (works with accumulate's sequential guarantee)
    ASSERT_EQ(utils_cpp::accumulate(std::vector<int> {1,2,3,4}, 0, [](int acc, int x){ return acc + x * 2; }), 20);

    // Test with list
    ASSERT_EQ(utils_cpp::accumulate(std::list<int> {5,10,15}), 30);
    ASSERT_EQ(utils_cpp::accumulate(std::list<int> {5,10,15}, 5), 35);

    // Test with strings (concatenation)
    ASSERT_EQ(utils_cpp::accumulate(std::vector<std::string> {"a", "b", "c"}, std::string("")), "abc");
    ASSERT_EQ(utils_cpp::accumulate(std::vector<std::string> {"a", "b", "c"}, std::string("start:")), "start:abc");

    // Test empty string container with custom defaultValue
    ASSERT_EQ(utils_cpp::accumulate(std::vector<std::string> {}, std::string("init"), std::plus<>(), {std::string("EMPTY")}), "EMPTY");

    // Test with doubles
    ASSERT_NEAR(utils_cpp::accumulate(std::vector<double> {1.5, 2.5, 3.0}), 7.0, 0.001);
    ASSERT_NEAR(utils_cpp::accumulate(std::vector<double> {1.5, 2.5, 3.0}, 0.5), 7.5, 0.001);
}

TEST(utils_cpp, ContainerUtilsTest_reduce)
{
    // Test basic sum with default init (0)
    ASSERT_EQ(utils_cpp::reduce(std::vector<int> {1,2,3,4}), 10);
    ASSERT_EQ(utils_cpp::reduce(std::vector<int> {1,2,3,4}, 0), 10);

    // Test sum with non-zero init
    ASSERT_EQ(utils_cpp::reduce(std::vector<int> {1,2,3,4}, 10), 20);

    // Test with empty container (default behavior - returns init)
    ASSERT_EQ(utils_cpp::reduce(std::vector<int> {}), 0);
    ASSERT_EQ(utils_cpp::reduce(std::vector<int> {}, 100), 100);

    // Test with empty container and custom defaultValue
    ASSERT_EQ(utils_cpp::reduce(std::vector<int> {}, 100, std::plus<>(), {-1}), -1);
    ASSERT_EQ(utils_cpp::reduce(std::vector<int> {}, 10, std::plus<>(), {999}), 999);

    // Test with non-empty container and defaultValue (defaultValue should be ignored)
    ASSERT_EQ(utils_cpp::reduce(std::vector<int> {1,2,3}, 0, std::plus<>(), {999}), 6);
    ASSERT_EQ(utils_cpp::reduce(std::vector<int> {1,2,3}, 10, std::plus<>(), {999}), 16);

    // Test with multiplication
    ASSERT_EQ(utils_cpp::reduce(std::vector<int> {1,2,3,4}, 1, std::multiplies<>()), 24);
    ASSERT_EQ(utils_cpp::reduce(std::vector<int> {2,3,4}, 10, std::multiplies<>()), 240);

    // Test with custom associative operation
    ASSERT_EQ(utils_cpp::reduce(std::vector<int> {1,2,3,4}, 0, std::plus<>()), 10);

    // Test with list
    ASSERT_EQ(utils_cpp::reduce(std::list<int> {5,10,15}), 30);
    ASSERT_EQ(utils_cpp::reduce(std::list<int> {5,10,15}, 5), 35);

    // Test with strings (concatenation)
    ASSERT_EQ(utils_cpp::reduce(std::vector<std::string> {"a", "b", "c"}, std::string("")), "abc");
    ASSERT_EQ(utils_cpp::reduce(std::vector<std::string> {"a", "b", "c"}, std::string("start:")), "start:abc");

    // Test empty string container with custom defaultValue
    ASSERT_EQ(utils_cpp::reduce(std::vector<std::string> {}, std::string("init"), std::plus<>(), {std::string("EMPTY")}), "EMPTY");

    // Test with doubles
    ASSERT_NEAR(utils_cpp::reduce(std::vector<double> {1.5, 2.5, 3.0}), 7.0, 0.001);
    ASSERT_NEAR(utils_cpp::reduce(std::vector<double> {1.5, 2.5, 3.0}, 0.5), 7.5, 0.001);

    // Test single element
    ASSERT_EQ(utils_cpp::reduce(std::vector<int> {42}), 42);
    ASSERT_EQ(utils_cpp::reduce(std::vector<int> {42}, 10), 52);

    // Test single element with defaultValue (should be ignored)
    ASSERT_EQ(utils_cpp::reduce(std::vector<int> {42}, 10, std::plus<>(), {999}), 52);
}

TEST(utils_cpp, ContainerUtilsTest_transform_reduce)
{
    // Test basic transform and reduce (sum of doubled values)
    ASSERT_EQ(utils_cpp::transform_reduce(std::vector<int> {1,2,3,4}, 0, std::plus<>(), [](int x){ return x * 2; }), 20);

    // Test with non-zero init
    ASSERT_EQ(utils_cpp::transform_reduce(std::vector<int> {1,2,3,4}, 10, std::plus<>(), [](int x){ return x * 2; }), 30);

    // Test with empty container
    ASSERT_EQ(utils_cpp::transform_reduce(std::vector<int> {}, 0, std::plus<>(), [](int x){ return x * 2; }), 0);
    ASSERT_EQ(utils_cpp::transform_reduce(std::vector<int> {}, 100, std::plus<>(), [](int x){ return x * 2; }), 100);

    // Test with empty container and custom defaultValue
    ASSERT_EQ(utils_cpp::transform_reduce(std::vector<int> {}, 100, std::plus<>(), [](int x){ return x * 2; }, {-1}), -1);

    // Test with non-empty container and defaultValue (should be ignored)
    ASSERT_EQ(utils_cpp::transform_reduce(std::vector<int> {1,2,3}, 0, std::plus<>(), [](int x){ return x * 2; }, {999}), 12);

    // Test with multiplication reduce
    ASSERT_EQ(utils_cpp::transform_reduce(std::vector<int> {1,2,3,4}, 1, std::multiplies<>(), [](int x){ return x + 1; }), 120);
    // (1+1) * (2+1) * (3+1) * (4+1) = 2 * 3 * 4 * 5 = 120

    // Test with list
    ASSERT_EQ(utils_cpp::transform_reduce(std::list<int> {5,10,15}, 0, std::plus<>(), [](int x){ return x / 5; }), 6);
    // (5/5) + (10/5) + (15/5) = 1 + 2 + 3 = 6

    // Test with strings (transform to length, sum lengths)
    ASSERT_EQ(utils_cpp::transform_reduce(std::vector<std::string> {"a", "bb", "ccc"}, 0, std::plus<>(), [](const auto& s){ return s.size(); }), 6);

    // Test with doubles
    ASSERT_NEAR(utils_cpp::transform_reduce(std::vector<double> {1.0, 2.0, 3.0}, 0.0, std::plus<>(), [](double x){ return x * x; }), 14.0, 0.001);
    // 1^2 + 2^2 + 3^2 = 1 + 4 + 9 = 14

    // Test single element
    ASSERT_EQ(utils_cpp::transform_reduce(std::vector<int> {5}, 0, std::plus<>(), [](int x){ return x * 10; }), 50);
}

#ifdef UTILS_CPP_HAS_EXECUTION_POLICIES
TEST(utils_cpp, ContainerUtilsTest_ExecutionPolicy)
{
    // Test find with execution policy
    {
        auto result = utils_cpp::find(std::execution::seq, std::vector<int> {1,2,3,4}, 2);
        ASSERT_TRUE(result.has_value());
        ASSERT_EQ(*result, 2);
        ASSERT_EQ(result.index(), 1);
    }

    // Test find_if with execution policy
    {
        auto result = utils_cpp::find_if(std::execution::seq, std::vector<int> {1,2,3,4}, [](int x){ return x > 2; });
        ASSERT_TRUE(result.has_value());
        ASSERT_EQ(*result, 3);
    }

    // Test find_ref with execution policy
    {
        std::vector<int> container {1,2,3,4};
        auto result = utils_cpp::find_ref(std::execution::seq, container, 2);
        ASSERT_TRUE(result.has_value());
        ASSERT_EQ(*result, 2);
        *result = 20;
        ASSERT_EQ(container[1], 20);
    }

    // Test find_if_ref with execution policy
    {
        std::vector<int> container {1,2,3,4};
        auto result = utils_cpp::find_if_ref(std::execution::seq, container, [](int x){ return x > 2; });
        ASSERT_TRUE(result.has_value());
        ASSERT_EQ(*result, 3);
        *result = 30;
        ASSERT_EQ(container[2], 30);
    }

    // Test find_cref with execution policy
    {
        const std::vector<int> container {1,2,3,4};
        auto result = utils_cpp::find_cref(std::execution::seq, container, 2);
        ASSERT_TRUE(result.has_value());
        ASSERT_EQ(*result, 2);
    }

    // Test find_if_cref with execution policy
    {
        const std::vector<int> container {1,2,3,4};
        auto result = utils_cpp::find_if_cref(std::execution::seq, container, [](int x){ return x > 2; });
        ASSERT_TRUE(result.has_value());
        ASSERT_EQ(*result, 3);
    }

    // Test contains with execution policy
    {
        ASSERT_TRUE(utils_cpp::contains(std::execution::seq, std::vector<int> {1,2,3,4}, 2));
        ASSERT_FALSE(utils_cpp::contains(std::execution::seq, std::vector<int> {1,2,3,4}, 10));
    }

    // Test contains_if with execution policy
    {
        ASSERT_TRUE(utils_cpp::contains_if(std::execution::seq, std::vector<int> {1,2,3,4}, [](int x){ return x > 2; }));
        ASSERT_FALSE(utils_cpp::contains_if(std::execution::seq, std::vector<int> {1,2,3,4}, [](int x){ return x > 10; }));
    }

    // Test count with execution policy
    {
        ASSERT_EQ(utils_cpp::count(std::execution::seq, std::vector<int> {1,2,2,3,2,4}, 2), 3);
        ASSERT_EQ(utils_cpp::count(std::execution::seq, std::vector<int> {1,2,3,4}, 10), 0);
    }

    // Test count_if with execution policy
    {
        ASSERT_EQ(utils_cpp::count_if(std::execution::seq, std::vector<int> {1,2,3,4,5,6}, [](int x){ return x % 2 == 0; }), 3);
    }

    // Test all_of_if with execution policy
    {
        ASSERT_TRUE(utils_cpp::all_of_if(std::execution::seq, std::vector<int> {2,4,6,8}, [](int x){ return x % 2 == 0; }));
        ASSERT_FALSE(utils_cpp::all_of_if(std::execution::seq, std::vector<int> {2,3,4,6}, [](int x){ return x % 2 == 0; }));
    }

    // Test any_of_if with execution policy
    {
        ASSERT_TRUE(utils_cpp::any_of_if(std::execution::seq, std::vector<int> {1,3,5,6}, [](int x){ return x % 2 == 0; }));
        ASSERT_FALSE(utils_cpp::any_of_if(std::execution::seq, std::vector<int> {1,3,5,7}, [](int x){ return x % 2 == 0; }));
    }

    // Test none_of_if with execution policy
    {
        ASSERT_TRUE(utils_cpp::none_of_if(std::execution::seq, std::vector<int> {1,3,5,7}, [](int x){ return x % 2 == 0; }));
        ASSERT_FALSE(utils_cpp::none_of_if(std::execution::seq, std::vector<int> {1,3,4,5}, [](int x){ return x % 2 == 0; }));
    }

    // Test accumulate with execution policy
    {
        ASSERT_EQ(utils_cpp::accumulate(std::execution::seq, std::vector<int> {1,2,3,4}), 10);
        ASSERT_EQ(utils_cpp::accumulate(std::execution::seq, std::vector<int> {1,2,3,4}, 10), 20);

        // Test with defaultValue
        ASSERT_EQ(utils_cpp::accumulate(std::execution::seq, std::vector<int> {}, 100, std::plus<>(), {-1}), -1);
        ASSERT_EQ(utils_cpp::accumulate(std::execution::seq, std::vector<int> {1,2,3}, 0, std::plus<>(), {999}), 6);
    }

    // Test reduce with execution policy
    {
        ASSERT_EQ(utils_cpp::reduce(std::execution::seq, std::vector<int> {1,2,3,4}), 10);
        ASSERT_EQ(utils_cpp::reduce(std::execution::seq, std::vector<int> {1,2,3,4}, 10), 20);
        ASSERT_EQ(utils_cpp::reduce(std::execution::seq, std::vector<int> {}), 0);

        // Test with defaultValue
        ASSERT_EQ(utils_cpp::reduce(std::execution::seq, std::vector<int> {}, 100, std::plus<>(), {-1}), -1);
        ASSERT_EQ(utils_cpp::reduce(std::execution::seq, std::vector<int> {1,2,3}, 0, std::plus<>(), {999}), 6);
    }

    // Test transform_reduce with execution policy
    {
        ASSERT_EQ(utils_cpp::transform_reduce(std::execution::seq, std::vector<int> {1,2,3,4}, 0, std::plus<>(), [](int x){ return x * 2; }), 20);
        ASSERT_EQ(utils_cpp::transform_reduce(std::execution::seq, std::vector<int> {1,2,3,4}, 10, std::plus<>(), [](int x){ return x * 2; }), 30);

        // Test with defaultValue
        ASSERT_EQ(utils_cpp::transform_reduce(std::execution::seq, std::vector<int> {}, 100, std::plus<>(), [](int x){ return x * 2; }, {-1}), -1);
        ASSERT_EQ(utils_cpp::transform_reduce(std::execution::seq, std::vector<int> {1,2,3}, 0, std::plus<>(), [](int x){ return x * 2; }, {999}), 12);
    }

    // Test transform with execution policy
    {
        auto result = utils_cpp::transform(std::execution::seq, std::vector<int> {1,2,3}, [](int v){ return v * 2; });
        ASSERT_EQ(result, (std::vector<int>{2, 4, 6}));
    }

    // Test index_of with execution policy
    {
        ASSERT_EQ(utils_cpp::index_of(std::execution::seq, std::vector<int> {1,2,3,4}, 3), 2);
        ASSERT_FALSE(utils_cpp::index_of(std::execution::seq, std::vector<int> {1,2,3,4}, 10));
    }

    // Test index_of_if with execution policy
    {
        ASSERT_EQ(utils_cpp::index_of_if(std::execution::seq, std::vector<int> {1,2,3,4}, [](int x){ return x > 2; }), 2);
    }
}
#endif

TEST(utils_cpp, ContainerUtilsTest_index_of)
{
    ASSERT_EQ(utils_cpp::index_of(std::vector<int> {1,2,3,4}, 2), 1);
    ASSERT_EQ(utils_cpp::index_of_if(std::vector<int> {1,2,3,4}, [](auto x){ return x == 2; }), 1);
    ASSERT_FALSE(utils_cpp::index_of(std::vector<int> {1,2,3,4}, 10));
    ASSERT_FALSE(utils_cpp::index_of_if(std::vector<int> {1,2,3,4}, [](auto x){ return x == 10; }));
}

TEST(utils_cpp, ContainerUtilsTest_copy_if)
{
    ASSERT_EQ(utils_cpp::copy_if(std::vector<int> {1,2,3,4}, [](auto v){ return v % 2; }), (std::vector<int>{1, 3}));
    ASSERT_EQ(utils_cpp::copy_if(std::vector<int> {1,2,3,4}, [](size_t index, const auto& v){ return index > 1 && v % 2; }), (std::vector<int>{3}));
    ASSERT_EQ(utils_cpp::copy_if<std::list>(std::vector<int> {1,2,3,4}, [](int v){ return v % 2; }), (std::list<int>{1, 3}));
}

TEST(utils_cpp, ContainerUtilsTest_transform)
{
    ASSERT_EQ(utils_cpp::transform(std::vector<int> {1,2,3}, [](auto v){ return std::to_string(v); }),                 (std::vector<std::string>{"1", "2", "3"}));
    ASSERT_EQ(utils_cpp::transform<std::list>(std::vector<int> {1,2,3}, [](int v){ return std::to_string(v); }),       (std::list<std::string>{"1", "2", "3"}));
    ASSERT_EQ(utils_cpp::transform(std::vector<int> {0,0,0}, [](size_t index, auto){ return std::to_string(index); }), (std::vector<std::string>{"0", "1", "2"}));
}

TEST(utils_cpp, ContainerUtilsTest_transform_reserve)
{
    // Reservable
    struct MyVector : std::vector<int> {
    public:
        using std::vector<int>::vector;
        void reserve(size_t) { m_reserved = true; }
        bool reserved() const { return m_reserved; }
    private:
        bool m_reserved = false;
    };

    auto result1 = utils_cpp::copy_if_transform<MyVector>(std::vector<int> {1,2,3}, [](const auto&){ return true; }, [](auto v){ return v*2; });
    ASSERT_EQ(result1, (MyVector{2, 4, 6}));
    ASSERT_FALSE(result1.reserved());

    auto result2 = utils_cpp::transform<MyVector>(std::vector<int> {1,2,3}, [](auto v){ return v*2; });
    ASSERT_EQ(result2, (MyVector{2, 4, 6}));
    ASSERT_TRUE(result2.reserved());

    // Not reservable
    auto result3 = utils_cpp::transform<std::set>(std::vector<int> {1,2,3}, [](auto v){ return v*2; });
    ASSERT_TRUE(utils_cpp::contains_set(result3, 2));
    ASSERT_TRUE(utils_cpp::contains_set(result3, 4));
    ASSERT_TRUE(utils_cpp::contains_set(result3, 6));
}

TEST(utils_cpp, ContainerUtilsTest_copy_if_transform)
{
    ASSERT_EQ(utils_cpp::copy_if_transform(
                  std::vector<int> {1,2,3,4},
                  [](int v) { return v % 2; },
                  [](int v){ return std::to_string(v); }),
              (std::vector<std::string>{"1", "3"}));

    ASSERT_EQ(utils_cpp::copy_if_transform<std::list>(
                  std::vector<int> {1,2,3,4},
                  [](size_t i, int v) { return i > 1 && v % 2; },
                  [](int v) { return std::to_string(v); }),
              (std::list<std::string>{"3"}));

    ASSERT_EQ(utils_cpp::copy_if_transform(
                  std::vector<int> {0,0,0,0},
                  [](int) { return true; },
                  [](size_t index, int){ return std::to_string(index); }),
              (std::vector<std::string>{"0", "1", "2", "3"}));
}

TEST(utils_cpp, ContainerUtilsTest_erase_all_vec)
{
    {
        std::vector<int> container {2, 2, 4, 6, 7};
        utils_cpp::erase_all_vec(container, [](int v){ return v % 2; });
        ASSERT_EQ(container, (std::vector<int>{2, 2, 4, 6}));
    }

    {
        std::vector<int> container {2, 2, 4, 6, 7};
        utils_cpp::erase_all_vec(container, [](size_t index, auto){ return index % 2; });
        ASSERT_EQ(container, (std::vector<int>{2, 4, 7}));
    }

    {
        std::vector<int> container {2, 2, 4, 6, 7};
        utils_cpp::erase_all_vec(container, 2);
        ASSERT_EQ(container, (std::vector<int>{4, 6, 7}));
    }

    {
        std::vector<int> container {};
        utils_cpp::erase_all_vec(container, [](auto v){ return v > 10; });
        ASSERT_EQ(container, (std::vector<int>{}));
    }

    {
        std::vector<int> container {1};
        utils_cpp::erase_all_vec(container, [](auto v){ return v > 10; });
        ASSERT_EQ(container, (std::vector<int>{1}));
    }

    {
        std::vector<int> container {11};
        utils_cpp::erase_all_vec(container, [](auto v){ return v > 10; });
        ASSERT_EQ(container, (std::vector<int>{}));
    }

    {
        std::vector<int> container {11, 9};
        utils_cpp::erase_all_vec(container, [](auto v){ return v > 10; });
        ASSERT_EQ(container, (std::vector<int>{9}));
    }

    {
        std::vector<int> container {9, 11};
        utils_cpp::erase_all_vec(container, [](auto v){ return v > 10; });
        ASSERT_EQ(container, (std::vector<int>{9}));
    }
}

TEST(utils_cpp, ContainerUtilsTest_erase_all)
{
    {
        std::vector<int> container {2, 2, 4, 6, 7};
        utils_cpp::erase_all(container, [](int v){ return v % 2; });
        ASSERT_EQ(container, (std::vector<int>{2, 2, 4, 6}));
    }

    {
        std::vector<int> container {2, 2, 4, 6, 7};
        utils_cpp::erase_all(container, [](size_t index, auto){ return index % 2; });
        ASSERT_EQ(container, (std::vector<int>{2, 4, 7}));
    }

    {
        std::vector<int> container {2, 2, 4, 6, 7};
        utils_cpp::erase_all(container, 2);
        ASSERT_EQ(container, (std::vector<int>{4, 6, 7}));
    }

    {
        std::vector<int> container {};
        utils_cpp::erase_all(container, [](auto v){ return v > 10; });
        ASSERT_EQ(container, (std::vector<int>{}));
    }

    {
        std::vector<int> container {1};
        utils_cpp::erase_all(container, [](auto v){ return v > 10; });
        ASSERT_EQ(container, (std::vector<int>{1}));
    }

    {
        std::vector<int> container {11};
        utils_cpp::erase_all(container, [](auto v){ return v > 10; });
        ASSERT_EQ(container, (std::vector<int>{}));
    }

    {
        std::vector<int> container {11, 9};
        utils_cpp::erase_all(container, [](auto v){ return v > 10; });
        ASSERT_EQ(container, (std::vector<int>{9}));
    }

    {
        std::vector<int> container {9, 11};
        utils_cpp::erase_all(container, [](auto v){ return v > 10; });
        ASSERT_EQ(container, (std::vector<int>{9}));
    }
}

TEST(utils_cpp, ContainerUtilsTest_erase_one)
{
    {
        std::vector<int> container {2, 2, 4, 6, 7};
        utils_cpp::erase_one(container, [](int v){ return v % 2; });
        ASSERT_EQ(container, (std::vector<int>{2, 2, 4, 6}));
    }

    {
        std::vector<int> container {2, 2, 4, 6, 7};
        utils_cpp::erase_one(container, [](size_t index, auto){ return index % 2; });
        ASSERT_EQ(container, (std::vector<int>{2, 4, 6, 7}));
    }

    {
        std::vector<int> container {2, 2, 4, 6, 7};
        utils_cpp::erase_one(container, 2);
        ASSERT_EQ(container, (std::vector<int>{2, 4, 6, 7}));
    }

    {
        std::vector<int> container {};
        utils_cpp::erase_one(container, [](auto v){ return v > 10; });
        ASSERT_EQ(container, (std::vector<int>{}));
    }

    {
        std::vector<int> container {1};
        utils_cpp::erase_one(container, [](auto v){ return v > 10; });
        ASSERT_EQ(container, (std::vector<int>{1}));
    }

    {
        std::vector<int> container {11};
        utils_cpp::erase_one(container, [](auto v){ return v > 10; });
        ASSERT_EQ(container, (std::vector<int>{}));
    }

    {
        std::vector<int> container {11, 9};
        utils_cpp::erase_one(container, [](auto v){ return v > 10; });
        ASSERT_EQ(container, (std::vector<int>{9}));
    }

    {
        std::vector<int> container {9, 11};
        utils_cpp::erase_one(container, [](auto v){ return v > 10; });
        ASSERT_EQ(container, (std::vector<int>{9}));
    }
}

TEST(utils_cpp, ContainerUtilsTest_difference_sorted)
{
    std::vector<int> a {1, 2, 3, 4   };
    std::vector<int> b {   2, 3, 4, 5};
    std::set<int> c    {   5, 3, 2, 4};

    const auto addedToB = utils_cpp::difference_sorted(b, a);
    const auto removedFromB = utils_cpp::difference_sorted(a, b);
    ASSERT_EQ(addedToB, (std::vector<int>{5}));
    ASSERT_EQ(removedFromB, (std::vector<int>{1}));

    const auto addedToC = utils_cpp::difference_sorted(c, a); // set
    const auto removedFromC = utils_cpp::difference_sorted(a, c); // vector
    ASSERT_EQ(addedToC, (std::set<int>{5}));
    ASSERT_EQ(removedFromC, (std::vector<int>{1}));

    const auto addedToC2 = utils_cpp::difference_sorted<std::vector>(c, a);
    const auto removedFromC2 = utils_cpp::difference_sorted(a, c);
    ASSERT_EQ(addedToC2, (std::vector<int>{5}));
    ASSERT_EQ(removedFromC2, (std::vector<int>{1}));
}

TEST(utils_cpp, ContainerUtilsTest_difference_sorted_custom)
{
    struct MyStruct {
        int value {};

        MyStruct() = default;
        MyStruct(int value) : value(value) {}
        MyStruct(const MyStruct&) = default;

        bool operator==(const MyStruct& rhs) const { return value == rhs.value; }
        static bool cmp(const MyStruct& lhs, const MyStruct& rhs) { return lhs.value < rhs.value; }
    };

    struct Cmp {
        bool operator()(const MyStruct& lhs, const MyStruct& rhs) const { return lhs.value < rhs.value; }
    };


    std::vector<MyStruct> a   {1, 2, 3, 4   };
    std::vector<MyStruct> b   {   2, 3, 4, 5};
    std::set<MyStruct, Cmp> c {   5, 3, 2, 4};

    const auto addedToB = utils_cpp::difference_sorted(b, a, MyStruct::cmp);
    const auto removedFromB = utils_cpp::difference_sorted(a, b, MyStruct::cmp);
    ASSERT_EQ(addedToB, (std::vector<MyStruct>{5}));
    ASSERT_EQ(removedFromB, (std::vector<MyStruct>{1}));

    const auto addedToC = utils_cpp::difference_sorted(c, a, MyStruct::cmp); // set
    const auto removedFromC = utils_cpp::difference_sorted(a, c, MyStruct::cmp); // vector
    ASSERT_EQ(addedToC, (std::set<MyStruct, Cmp>{5}));
    ASSERT_EQ(removedFromC, (std::vector<MyStruct>{1}));

    const auto addedToC2 = utils_cpp::difference_sorted<std::vector>(c, a, MyStruct::cmp);
    const auto removedFromC2 = utils_cpp::difference_sorted(a, c, MyStruct::cmp);
    ASSERT_EQ(addedToC2, (std::vector<MyStruct>{5}));
    ASSERT_EQ(removedFromC2, (std::vector<MyStruct>{1}));
}

// ---- Test detailed_difference ----
namespace {

struct TestData
{
    std::vector<int> a;
    std::vector<int> b;
    size_t expectedAddCount {};
    size_t expectedRemoveCount {};
};

} // namespace

class ContainerUtils_detailed_difference : public ::testing::TestWithParam<TestData> {
protected:
    void SetUp() override { }
    void TearDown() override { }
};

TEST_P(ContainerUtils_detailed_difference, Test) {
    const auto& params = GetParam();
    const auto& a = params.a;
    const auto& b = params.b;
    std::vector<int> c = a;
    size_t addCount {};
    size_t removeCount {};

    auto adder = [&](auto srcItBegin, auto srcItEnd, int64_t insertionIndex){
        c.insert(c.begin() + insertionIndex, srcItBegin, srcItEnd);
        addCount++;
    };

    auto remover = [&](auto minIndex, int64_t maxIndex){
        static_assert(std::is_same_v<decltype(minIndex), std::vector<int>::difference_type>);
        c.erase(c.begin() + minIndex, c.begin() + maxIndex + 1);
        removeCount++;
    };

    utils_cpp::difference_sorted_detailed(a, b, adder, remover);
    EXPECT_EQ(c, b);
    EXPECT_EQ(addCount, params.expectedAddCount);
    EXPECT_EQ(removeCount, params.expectedRemoveCount);
}

INSTANTIATE_TEST_SUITE_P(
    ContainerUtils_detailed_difference,
    ContainerUtils_detailed_difference,
    ::testing::Values(
        TestData{{},                          {},                          0, 0}, // No changes
        TestData{{1},                         {1},                         0, 0},
        TestData{{1, 2},                      {1, 2},                      0, 0},
        TestData{{1, 2, 3},                   {1, 2, 3},                   0, 0},
        TestData{{},                          {1},                         1, 0}, // Insertions
        TestData{{1},                         {1, 2},                      1, 0},
        TestData{{1, 2},                      {1, 2, 3},                   1, 0},
        TestData{{1, 2},                      {1, 2, 3, 4},                1, 0},
        TestData{{},                          {1, 2},                      1, 0},
        TestData{{},                          {1, 2, 5},                   1, 0},
        TestData{{1},                         {},                          0, 1}, // Removals
        TestData{{1, 2},                      {1},                         0, 1},
        TestData{{1, 2, 3},                   {1, 2},                      0, 1},
        TestData{{1, 2, 3, 4},                {1, 2},                      0, 1},
        TestData{{1, 2},                      {},                          0, 1},
        TestData{{1, 2, 5},                   {},                          0, 1},
        TestData{{1},                         {0, 1, 2},                   2, 0}, // Several insertions
        TestData{{1, 3},                      {0, 1, 2, 3},                2, 0},
        TestData{{1, 3},                      {1, 2, 3, 4},                2, 0},
        TestData{{1, 3, 5, 7},                {0, 1, 2, 3, 4, 5, 6, 7, 8}, 5, 0},
        TestData{{1, 3, 7},                   {0, 1, 2, 3, 4, 5, 6, 7, 8}, 4, 0},
        TestData{{0, 1, 2},                   {1},                         0, 2}, // Several removals
        TestData{{0, 1, 2, 3},                {1, 3},                      0, 2},
        TestData{{1, 2, 3, 4},                {1, 3},                      0, 2},
        TestData{{0, 1, 2, 3, 4, 5, 6, 7, 8}, {1, 3, 5, 7},                0, 5},
        TestData{{0, 1, 2, 3, 4, 5, 6, 7, 8}, {1, 3, 7},                   0, 4},
        TestData{{1},                         {10},                        1, 1}, // One insertion and one removal
        TestData{{1, 2},                      {1, 10},                     1, 1},
        TestData{{1, 3},                      {0, 3},                      1, 1},
        TestData{{1, 3},                      {2, 3},                      1, 1},
        TestData{{1, 3, 5},                   {1, 3, 9},                   1, 1},
        TestData{{1, 3, 5},                   {1, 2, 5},                   1, 1},
        TestData{{1, 3, 5},                   {1, 4, 5},                   1, 1},
        TestData{{1, 3, 5},                   {0, 3, 5},                   1, 1},
        TestData{{1, 3, 5, 7},                {1, 3, 10},                  1, 1},
        TestData{{1, 3, 5, 7, 9},             {1, 3, 10},                  1, 1},
        TestData{{1, 2, 3},                   {0, 2, 3, 4},                2, 1}, // Several insertions and removals
        TestData{{1, 2, 3, 4, 5},             {6, 7, 8, 9, 10},            1, 1},
        TestData{{1, 3, 5, 7, 9},             {2, 4, 6, 8, 10},            5, 5}
    )
);

// ---- Test difference_sorted_detailed with changes ----

namespace {

struct Item {
    int sortKey {};
    int value {};

    Item() = default;
    Item(int k, int v) : sortKey{k}, value{v} {}

    bool operator==(const Item& rhs) const                    { return sortKey == rhs.sortKey && value == rhs.value; }
    static bool compareKeyLess(const Item& lhs, const Item& rhs) { return lhs.sortKey < rhs.sortKey; }
    static bool compareEq(const Item& lhs, const Item& rhs)   { return lhs == rhs; }
};

} // namespace

TEST(utils_cpp, ContainerUtilsTest_difference_sorted_detailed_with_changes_disabled)
{
    // Test with enableCompareEq = false - changed handler should NOT be called
    std::vector<Item> a {{1, 10}, {2, 20}, {3, 30}};
    std::vector<Item> b {{1, 100}, {2, 20}, {3, 300}};  // Items 1 and 3 have different values
    std::vector<Item> c = a;
    size_t addCount = 0;
    size_t removeCount = 0;
    size_t changeCount = 0;

    auto adder = [&](auto /*srcItBegin*/, auto /*srcItEnd*/, int64_t /*insertionIndex*/){ addCount++; };
    auto remover = [&](auto /*minIndex*/, int64_t /*maxIndex*/){ removeCount++; };
    auto changer = [&](auto /*srcItBegin*/, auto /*srcItEnd*/, int64_t /*dstStartIndex*/){ changeCount++; };

    // Call with enableCompareEq = false - changes should NOT be detected
    utils_cpp::difference_sorted_detailed(a, b, adder, remover, changer, false, Item::compareKeyLess);

    // Without change detection, c won't match b (items at indices 0 and 2 remain unchanged)
    EXPECT_EQ(addCount, 0u);
    EXPECT_EQ(removeCount, 0u);
    EXPECT_EQ(changeCount, 0u);
    EXPECT_NE(c, b);
}

TEST(utils_cpp, ContainerUtilsTest_difference_sorted_detailed_with_changes_enabled)
{
    // Test with enableCompareEq = true - changed handler SHOULD be called and update container
    std::vector<Item> a {{1, 10}, {2, 20}, {3, 30}};
    std::vector<Item> b {{1, 100}, {2, 20}, {3, 300}};  // Items 1 and 3 have different values
    std::vector<Item> c = a;
    size_t addCount = 0;
    size_t removeCount = 0;
    size_t changeCount = 0;

    auto adder = [&](auto /*srcItBegin*/, auto /*srcItEnd*/, int64_t /*insertionIndex*/) { addCount++; };
    auto remover = [&](auto /*minIndex*/, int64_t /*maxIndex*/) { removeCount++; };

    auto changer = [&](auto srcItBegin, auto srcItEnd, int64_t dstStartIndex){
        changeCount++;
        auto dstIt = c.begin() + dstStartIndex;
        for (auto srcIt = srcItBegin; srcIt != srcItEnd; ++srcIt, ++dstIt) {
            *dstIt = *srcIt;
        }
    };

    // Call with enableCompareEq = true
    utils_cpp::difference_sorted_detailed(a, b, adder, remover, changer, true, Item::compareKeyLess, Item::compareEq);

    // With change detection enabled, c should match b
    EXPECT_EQ(addCount, 0u);
    EXPECT_EQ(removeCount, 0u);
    EXPECT_EQ(changeCount, 2u);
    EXPECT_EQ(c, b);
}

TEST(utils_cpp, ContainerUtilsTest_difference_sorted_detailed_mixed_operations)
{
    // Test with additions, removals, AND changes in the same call
    std::vector<Item> a {{1, 10}, {2, 20}, {4, 40}, {5, 50}};
    std::vector<Item> b {{1, 100}, {2, 20}, {3, 30}, {5, 500}};
    std::vector<Item> c = a;
    // Changes: item 1 (index 0), item 5 (index 3->3)
    // Additions: item 3
    // Removals: item 4

    auto adder = [&](auto srcItBegin, auto srcItEnd, int64_t insertionIndex){
        c.insert(c.begin() + insertionIndex, srcItBegin, srcItEnd);
    };

    auto remover = [&](auto minIndex, int64_t maxIndex){
        c.erase(c.begin() + minIndex, c.begin() + maxIndex + 1);
    };

    auto changer = [&](auto srcItBegin, auto srcItEnd, int64_t dstStartIndex){
        auto dstIt = c.begin() + dstStartIndex;
        for (auto srcIt = srcItBegin; srcIt != srcItEnd; ++srcIt, ++dstIt) {
            *dstIt = *srcIt;
        }
    };

    utils_cpp::difference_sorted_detailed(a, b, adder, remover, changer, true, Item::compareKeyLess);

    EXPECT_EQ(c, b);
}

TEST(utils_cpp, ContainerUtilsTest_difference_sorted_detailed_no_changes_truly_equal)
{
    // Test when elements are sort-equal AND truly equal - no handler calls needed
    std::vector<Item> a {{1, 10}, {2, 20}, {3, 30}};
    std::vector<Item> b {{1, 10}, {2, 20}, {3, 30}};
    std::vector<Item> c = a;
    size_t addCount = 0;
    size_t removeCount = 0;
    size_t changeCount = 0;

    auto adder = [&](auto /*srcItBegin*/, auto /*srcItEnd*/, int64_t /*insertionIndex*/){ addCount++; };
    auto remover = [&](auto /*minIndex*/, int64_t /*maxIndex*/){ removeCount++; };
    auto changer = [&](auto /*srcItBegin*/, auto /*srcItEnd*/, int64_t /*dstStartIndex*/){ changeCount++; };

    utils_cpp::difference_sorted_detailed(a, b, adder, remover, changer, true, Item::compareKeyLess);

    EXPECT_EQ(addCount, 0u);
    EXPECT_EQ(removeCount, 0u);
    EXPECT_EQ(changeCount, 0u);
    EXPECT_EQ(c, b);
}

TEST(utils_cpp, ContainerUtilsTest_difference_sorted_detailed_all_changed)
{
    // Test when all elements are changed - should be batched into one call
    std::vector<Item> a {{1, 10}, {2, 20}, {3, 30}};
    std::vector<Item> b {{1, 100}, {2, 200}, {3, 300}};
    std::vector<Item> c = a;
    size_t addCount = 0;
    size_t removeCount = 0;
    size_t changeCount = 0;

    auto adder = [&](auto /*srcItBegin*/, auto /*srcItEnd*/, int64_t /*insertionIndex*/){ addCount++; };
    auto remover = [&](auto /*minIndex*/, int64_t /*maxIndex*/){ removeCount++; };
    auto changer = [&](auto srcItBegin, auto srcItEnd, int64_t dstStartIndex){
        changeCount++;
        // All three should be batched into one call
        EXPECT_EQ(dstStartIndex, 0);
        EXPECT_EQ(std::distance(srcItBegin, srcItEnd), 3);
        auto dstIt = c.begin() + dstStartIndex;
        for (auto srcIt = srcItBegin; srcIt != srcItEnd; ++srcIt, ++dstIt) {
            *dstIt = *srcIt;
        }
    };

    utils_cpp::difference_sorted_detailed(a, b, adder, remover, changer, true, Item::compareKeyLess);

    EXPECT_EQ(addCount, 0u);
    EXPECT_EQ(removeCount, 0u);
    EXPECT_EQ(changeCount, 1u);  // Should be called once for all three items (batched)
    EXPECT_EQ(c, b);
}

TEST(utils_cpp, ContainerUtilsTest_difference_sorted_detailed_empty_containers)
{
    // Test with empty containers
    std::vector<Item> a {};
    std::vector<Item> b {};
    std::vector<Item> c = a;
    size_t addCount = 0;
    size_t removeCount = 0;
    size_t changeCount = 0;

    auto adder = [&](auto /*srcItBegin*/, auto /*srcItEnd*/, int64_t /*insertionIndex*/){ addCount++; };
    auto remover = [&](auto /*minIndex*/, int64_t /*maxIndex*/){ removeCount++; };
    auto changer = [&](auto /*srcItBegin*/, auto /*srcItEnd*/, int64_t /*dstStartIndex*/){ changeCount++; };

    utils_cpp::difference_sorted_detailed(a, b, adder, remover, changer, true, Item::compareKeyLess);

    EXPECT_EQ(addCount, 0u);
    EXPECT_EQ(removeCount, 0u);
    EXPECT_EQ(changeCount, 0u);
    EXPECT_EQ(c, b);
}

TEST(utils_cpp, ContainerUtilsTest_difference_sorted_detailed_consecutive_changes_batched)
{
    // Test that consecutive changed elements are batched together
    std::vector<Item> a {{1, 10}, {2, 20}, {3, 30}, {4, 40}, {5, 50}, {6, 60}};
    std::vector<Item> b {{1, 100}, {2, 200}, {3, 300}, {4, 40}, {5, 500}, {6, 600}};
    // Changes: items 1,2,3 should be batched (indices 0-2), then item 5,6 should be batched (indices 4-5)
    size_t addCount = 0;
    size_t removeCount = 0;
    std::vector<std::tuple<int64_t, int64_t, int64_t>> changedIndices; // dstStartIndex, dstEndIndex, srcStartIndex

    auto adder = [&](auto /*srcItBegin*/, auto /*srcItEnd*/, int64_t /*insertionIndex*/){ addCount++; };
    auto remover = [&](auto /*minIndex*/, int64_t /*maxIndex*/){ removeCount++; };
    auto changer = [&](auto srcItBegin, auto srcItEnd, int64_t dstStartIndex){
        auto count = std::distance(srcItBegin, srcItEnd);
        auto dstEndIndex = dstStartIndex + count - 1;
        auto srcStartIndex = std::distance(b.cbegin(), srcItBegin);
        changedIndices.push_back({dstStartIndex, dstEndIndex, srcStartIndex});
    };

    utils_cpp::difference_sorted_detailed(a, b, adder, remover, changer, true, Item::compareKeyLess);

    EXPECT_EQ(addCount, 0u);
    EXPECT_EQ(removeCount, 0u);
    ASSERT_EQ(changedIndices.size(), 2u); // Should be called twice: once for [0-2], once for [4-5]

    EXPECT_EQ(changedIndices[0], std::make_tuple(0, 2, 0)); // dst[0-2] from src[0-2]
    EXPECT_EQ(changedIndices[1], std::make_tuple(4, 5, 4)); // dst[4-5] from src[4-5]
}

TEST(utils_cpp, ContainerUtilsTest_difference_sorted_detailed_intensive_corner_cases)
{
    // Comprehensive test with many operations and corner cases
    // a: 1  2  3  4  5  6  7  8  9  10 11 12 13  14 15  16 17  18 19 20
    // b: 0  1  2* 4* 5* 7  8  9* 11 12 13 14 15* 16 17* 18 19* 21 22 23
    //    A  =  C  R  C  R  =  C  R  =  =  =  C   =  C   =  C   R  A  A  A
    // Legend: = same, C changed, R removed, A added

    std::vector<Item> a {
        {1, 10},  {2, 20},  {3, 30},  {4, 40},  {5, 50},   // 0-4
        {6, 60},  {7, 70},  {8, 80},  {9, 90},  {10, 100}, // 5-9
        {11, 110}, {12, 120}, {13, 130}, {14, 140}, {15, 150}, // 10-14
        {16, 160}, {17, 170}, {18, 180}, {19, 190}, {20, 200}  // 15-19
    };

    std::vector<Item> b {
        {0, 0},    {1, 10},  {2, 22},  {4, 44},  {5, 55},   // 0-4: Add at start, same, change, remove, change
        {7, 70},   {8, 80},  {9, 99},  {11, 110}, {12, 120}, // 5-9: remove, same, change, remove, same
        {13, 130}, {14, 140}, {15, 155}, {16, 160}, {17, 177}, // 10-14: same, same, change, same, change
        {18, 180}, {19, 199}, {21, 210}, {22, 220}, {23, 230}  // 15-19: same, change, add, add, add at end
    };

    std::vector<Item> c = a;
    std::vector<std::tuple<int64_t, int64_t, int64_t>> additions;
    std::vector<std::tuple<int64_t, int64_t>> removals;
    std::vector<std::tuple<int64_t, int64_t, int64_t>> changes; // dstStartIndex, dstEndIndex, srcStartIndex

    auto adder = [&](auto srcItBegin, auto srcItEnd, int64_t insertionIndex){
        c.insert(c.begin() + insertionIndex, srcItBegin, srcItEnd);
        additions.push_back({insertionIndex, insertionIndex + std::distance(srcItBegin, srcItEnd) - 1, -1});
    };

    auto remover = [&](auto minIndex, int64_t maxIndex){
        c.erase(c.begin() + minIndex, c.begin() + maxIndex + 1);
        removals.push_back({minIndex, maxIndex});
    };

    auto changer = [&](auto srcItBegin, auto srcItEnd, int64_t dstStartIndex){
        auto dstIt = c.begin() + dstStartIndex;
        for (auto srcIt = srcItBegin; srcIt != srcItEnd; ++srcIt, ++dstIt) {
            *dstIt = *srcIt;
        }
        auto count = std::distance(srcItBegin, srcItEnd);
        auto dstEndIndex = dstStartIndex + count - 1;
        auto srcStartIndex = std::distance(b.cbegin(), srcItBegin);
        changes.push_back({dstStartIndex, dstEndIndex, srcStartIndex});
    };

    utils_cpp::difference_sorted_detailed(a, b, adder, remover, changer, true, Item::compareKeyLess);

    // Verify final result matches
    EXPECT_EQ(c, b);

    // Verify we had operations
    EXPECT_FALSE(additions.empty());
    EXPECT_FALSE(removals.empty());
    EXPECT_FALSE(changes.empty());

    // Detailed verification of operations
    // Additions: 0 at start (item {0,0}), and 21,22,23 at end
    EXPECT_GE(additions.size(), 2u);

    // Removals: items 3, 6, 10, 20 (sortKeys that are missing in b)
    EXPECT_GE(removals.size(), 2u);

    // Changes: items 2, 4, 5, 9, 15, 17, 19 (sortKeys that exist but values differ)
    // Should be batched into groups: [2], [4,5], [9], [15], [17], [19]
    EXPECT_GE(changes.size(), 3u);
}

TEST(utils_cpp, ContainerUtilsTest_difference_sorted_detailed_all_operations_at_boundaries)
{
    // Test operations specifically at boundaries (start and end)

    // Scenario 1: Changes at start
    {
        std::vector<Item> a {{1, 10}, {2, 20}, {3, 30}};
        std::vector<Item> b {{1, 11}, {2, 22}, {3, 30}};
        std::vector<Item> c = a;

        auto adder = [&](auto srcItBegin, auto srcItEnd, int64_t insertionIndex){
            c.insert(c.begin() + insertionIndex, srcItBegin, srcItEnd);
        };
        auto remover = [&](auto minIndex, int64_t maxIndex){
            c.erase(c.begin() + minIndex, c.begin() + maxIndex + 1);
        };
        auto changer = [&](auto srcItBegin, auto srcItEnd, int64_t dstStartIndex){
            auto dstIt = c.begin() + dstStartIndex;
            for (auto srcIt = srcItBegin; srcIt != srcItEnd; ++srcIt, ++dstIt) {
                *dstIt = *srcIt;
            }
        };

        utils_cpp::difference_sorted_detailed(a, b, adder, remover, changer, true, Item::compareKeyLess);
        EXPECT_EQ(c, b);
    }

    // Scenario 2: Changes at end
    {
        std::vector<Item> a {{1, 10}, {2, 20}, {3, 30}};
        std::vector<Item> b {{1, 10}, {2, 22}, {3, 33}};
        std::vector<Item> c = a;

        auto adder = [&](auto srcItBegin, auto srcItEnd, int64_t insertionIndex){
            c.insert(c.begin() + insertionIndex, srcItBegin, srcItEnd);
        };
        auto remover = [&](auto minIndex, int64_t maxIndex){
            c.erase(c.begin() + minIndex, c.begin() + maxIndex + 1);
        };
        auto changer = [&](auto srcItBegin, auto srcItEnd, int64_t dstStartIndex){
            auto dstIt = c.begin() + dstStartIndex;
            for (auto srcIt = srcItBegin; srcIt != srcItEnd; ++srcIt, ++dstIt) {
                *dstIt = *srcIt;
            }
        };

        utils_cpp::difference_sorted_detailed(a, b, adder, remover, changer, true, Item::compareKeyLess);
        EXPECT_EQ(c, b);
    }

    // Scenario 3: Additions at start
    {
        std::vector<Item> a {{3, 30}, {4, 40}};
        std::vector<Item> b {{1, 10}, {2, 20}, {3, 30}, {4, 40}};
        std::vector<Item> c = a;

        auto adder = [&](auto srcItBegin, auto srcItEnd, int64_t insertionIndex){
            c.insert(c.begin() + insertionIndex, srcItBegin, srcItEnd);
        };
        auto remover = [&](auto minIndex, int64_t maxIndex){
            c.erase(c.begin() + minIndex, c.begin() + maxIndex + 1);
        };
        auto changer = [&](auto srcItBegin, auto srcItEnd, int64_t dstStartIndex){
            auto dstIt = c.begin() + dstStartIndex;
            for (auto srcIt = srcItBegin; srcIt != srcItEnd; ++srcIt, ++dstIt) {
                *dstIt = *srcIt;
            }
        };

        utils_cpp::difference_sorted_detailed(a, b, adder, remover, changer, true, Item::compareKeyLess);
        EXPECT_EQ(c, b);
    }

    // Scenario 4: Removals from start
    {
        std::vector<Item> a {{1, 10}, {2, 20}, {3, 30}, {4, 40}};
        std::vector<Item> b {{3, 30}, {4, 40}};
        std::vector<Item> c = a;

        auto adder = [&](auto srcItBegin, auto srcItEnd, int64_t insertionIndex){
            c.insert(c.begin() + insertionIndex, srcItBegin, srcItEnd);
        };
        auto remover = [&](auto minIndex, int64_t maxIndex){
            c.erase(c.begin() + minIndex, c.begin() + maxIndex + 1);
        };
        auto changer = [&](auto srcItBegin, auto srcItEnd, int64_t dstStartIndex){
            auto dstIt = c.begin() + dstStartIndex;
            for (auto srcIt = srcItBegin; srcIt != srcItEnd; ++srcIt, ++dstIt) {
                *dstIt = *srcIt;
            }
        };

        utils_cpp::difference_sorted_detailed(a, b, adder, remover, changer, true, Item::compareKeyLess);
        EXPECT_EQ(c, b);
    }

    // Scenario 5: Complex - all operations mixed
    {
        std::vector<Item> a {{2, 20}, {3, 30}, {5, 50}, {6, 60}, {8, 80}};
        std::vector<Item> b {{1, 10}, {2, 22}, {4, 40}, {5, 50}, {7, 70}, {8, 88}, {9, 90}};
        // Add 1 at start, change 2, remove 3, add 4, keep 5, remove 6, add 7, change 8, add 9 at end
        std::vector<Item> c = a;

        auto adder = [&](auto srcItBegin, auto srcItEnd, int64_t insertionIndex){
            c.insert(c.begin() + insertionIndex, srcItBegin, srcItEnd);
        };
        auto remover = [&](auto minIndex, int64_t maxIndex){
            c.erase(c.begin() + minIndex, c.begin() + maxIndex + 1);
        };
        auto changer = [&](auto srcItBegin, auto srcItEnd, int64_t dstStartIndex){
            auto dstIt = c.begin() + dstStartIndex;
            for (auto srcIt = srcItBegin; srcIt != srcItEnd; ++srcIt, ++dstIt) {
                *dstIt = *srcIt;
            }
        };

        utils_cpp::difference_sorted_detailed(a, b, adder, remover, changer, true, Item::compareKeyLess);
        EXPECT_EQ(c, b);
    }
}

TEST(utils_cpp, ContainerUtilsTest_difference_sorted_detailed_single_element_operations)
{
    // Test with single elements to verify minimal cases work

    // Single element, changed
    {
        std::vector<Item> a {{1, 10}};
        std::vector<Item> b {{1, 11}};
        std::vector<Item> c = a;

        auto adder = [&](auto srcItBegin, auto srcItEnd, int64_t insertionIndex){
            c.insert(c.begin() + insertionIndex, srcItBegin, srcItEnd);
        };
        auto remover = [&](auto minIndex, int64_t maxIndex){
            c.erase(c.begin() + minIndex, c.begin() + maxIndex + 1);
        };
        auto changer = [&](auto srcItBegin, auto srcItEnd, int64_t dstStartIndex){
            auto dstIt = c.begin() + dstStartIndex;
            for (auto srcIt = srcItBegin; srcIt != srcItEnd; ++srcIt, ++dstIt) {
                *dstIt = *srcIt;
            }
        };

        utils_cpp::difference_sorted_detailed(a, b, adder, remover, changer, true, Item::compareKeyLess);
        EXPECT_EQ(c, b);
    }

    // Single element, removed
    {
        std::vector<Item> a {{1, 10}};
        std::vector<Item> b {};
        std::vector<Item> c = a;

        auto adder = [&](auto srcItBegin, auto srcItEnd, int64_t insertionIndex){
            c.insert(c.begin() + insertionIndex, srcItBegin, srcItEnd);
        };
        auto remover = [&](auto minIndex, int64_t maxIndex){
            c.erase(c.begin() + minIndex, c.begin() + maxIndex + 1);
        };
        auto changer = [&](auto srcItBegin, auto srcItEnd, int64_t dstStartIndex){
            auto dstIt = c.begin() + dstStartIndex;
            for (auto srcIt = srcItBegin; srcIt != srcItEnd; ++srcIt, ++dstIt) {
                *dstIt = *srcIt;
            }
        };

        utils_cpp::difference_sorted_detailed(a, b, adder, remover, changer, true, Item::compareKeyLess);
        EXPECT_EQ(c, b);
    }

    // Single element, added
    {
        std::vector<Item> a {};
        std::vector<Item> b {{1, 10}};
        std::vector<Item> c = a;

        auto adder = [&](auto srcItBegin, auto srcItEnd, int64_t insertionIndex){
            c.insert(c.begin() + insertionIndex, srcItBegin, srcItEnd);
        };
        auto remover = [&](auto minIndex, int64_t maxIndex){
            c.erase(c.begin() + minIndex, c.begin() + maxIndex + 1);
        };
        auto changer = [&](auto srcItBegin, auto srcItEnd, int64_t dstStartIndex){
            auto dstIt = c.begin() + dstStartIndex;
            for (auto srcIt = srcItBegin; srcIt != srcItEnd; ++srcIt, ++dstIt) {
                *dstIt = *srcIt;
            }
        };

        utils_cpp::difference_sorted_detailed(a, b, adder, remover, changer, true, Item::compareKeyLess);
        EXPECT_EQ(c, b);
    }
}

// ---- ----

TEST(utils_cpp, ContainerUtilsTest_internal_random)
{
    constexpr int Count = 100;
    std::vector<int> randomNumbers1;
    std::vector<int> randomNumbers2;

    randomNumbers1.reserve(Count);
    randomNumbers2.reserve(Count);

    for (int i = 0; i < Count; ++i)
        randomNumbers1.push_back(utils_cpp::Internal::random(10));

    for (int i = 0; i < Count; ++i)
        randomNumbers2.push_back(utils_cpp::Internal::random(-10, 10));

    const auto first1 = randomNumbers1.front();
    const auto first2 = randomNumbers2.front();
    const auto hasAnother1 = utils_cpp::contains_if(randomNumbers1, [first1](const auto x){ return x != first1; });
    const auto hasAnother2 = utils_cpp::contains_if(randomNumbers2, [first2](const auto x){ return x != first2; });
    EXPECT_EQ(hasAnother1, true);
    EXPECT_EQ(hasAnother2, true);

    const auto hasNegative = utils_cpp::contains_if(randomNumbers2, [](const auto x){ return x < 0; });
    EXPECT_EQ(hasNegative, true);
}

TEST(utils_cpp, ContainerUtilsTest_generate)
{
    const auto vec = utils_cpp::generate<std::vector>(5, [i = 0]() mutable { return ++i; });
    ASSERT_EQ(vec, (std::vector<int>{1, 2, 3, 4, 5}));

    const auto vec2 = utils_cpp::generate_rnd<std::vector>(5, [](size_t rnd) { return rnd; });
    const auto first = vec2.front();
    const auto hasAnother = utils_cpp::contains_if(vec2, [first](const auto x){ return x != first; });
    EXPECT_EQ(hasAnother, true);
}

TEST(utils_cpp, ContainerUtilsTest_random_items)
{
    constexpr size_t Count = 500;
    const auto vec = utils_cpp::generate<std::vector>(Count, [i = 0]() mutable { return ++i; });
    const auto selection1 = utils_cpp::generate<std::vector>(Count, [&vec]() { return utils_cpp::random_item(vec); });
    const auto selection2 = utils_cpp::random_items(vec, Count);
    ASSERT_EQ(vec.size(), Count);
    ASSERT_EQ(selection1.size(), Count);
    ASSERT_EQ(selection2.size(), Count);

    EXPECT_NE(vec, selection1);
    EXPECT_NE(vec, selection2);
    EXPECT_NE(selection1, selection2);

    std::set<int> setSelection1 (selection1.cbegin(), selection1.cend());
    std::set<int> setSelection2 (selection2.cbegin(), selection2.cend());
    EXPECT_LT(setSelection1.size(), selection1.size());
    EXPECT_LT(setSelection2.size(), selection2.size());
}

TEST(utils_cpp, ContainerUtilsTest_random_items_unique)
{
    constexpr size_t Count = 500;
    const auto vec = utils_cpp::generate<std::vector>(Count, [i = 0]() mutable { return ++i; });
    const auto selection = utils_cpp::random_items_unique(vec, Count);
    ASSERT_EQ(vec.size(), Count);
    ASSERT_EQ(selection.size(), Count);

    EXPECT_NE(vec, selection);

    std::set<int> setSelection (selection.cbegin(), selection.cend());
    EXPECT_EQ(setSelection.size(), selection.size());
}

namespace {
struct Int {
    int value {};

    bool operator==(const Int& rhs) const { return rhs.value == value; }
    bool operator!=(const Int& rhs) const { return rhs.value != value; }

    static int weight(const Int& x)  { return x.value; }
    using Weight = Functor<weight>;
};
} // namespace

TEST(utils_cpp, ContainerUtilsTest_random_weighted_item_types)
{
    // Simple type
    constexpr size_t Count = 500;
    const std::vector<int> items {50, 1000, 1};
    const auto selection = utils_cpp::generate<std::vector>(Count, [&items](){ return utils_cpp::random_weighted_item(items); });
    const auto cnt1000 = std::count(selection.cbegin(), selection.cend(), 1000);
    const auto cnt50 = std::count(selection.cbegin(), selection.cend(), 50);
    EXPECT_GE(cnt1000, 400);
    EXPECT_GE(cnt50, 1);
    EXPECT_LE(cnt50, 50);

    // Custom type
    const std::vector<Int> itemsB {{50}, {1000}, {1}};
    const auto selectionB = utils_cpp::generate<std::vector>(Count, [&itemsB](){ return utils_cpp::random_weighted_item<Int::Weight>(itemsB); });
    const auto cnt1000b = std::count(selectionB.cbegin(), selectionB.cend(), Int{1000});
    const auto cnt50b = std::count(selectionB.cbegin(), selectionB.cend(), Int{50});
    EXPECT_GE(cnt1000b, 400);
    EXPECT_GE(cnt50b, 1);
    EXPECT_LE(cnt50b, 50);

    // Another weight functor
    utils_cpp::random_weighted_item(itemsB, [](const auto& x){ return x.value; });
}

TEST(utils_cpp, ContainerUtilsTest_random_weighted_item_stats1)
{
    constexpr size_t Count = 1000000; // 1M
    const std::vector<int> items {10, 5, 2, 1};

    { // Small container optimized
        const auto selection = utils_cpp::random_weighted_items(items, Count, {}, std::numeric_limits<size_t>::max());
        const auto percent10 = toDbl(std::count(selection.cbegin(), selection.cend(), 10)) * 100.0 / Count;
        const auto percent5 = toDbl(std::count(selection.cbegin(), selection.cend(), 5)) * 100.0 / Count;
        const auto percent2 = toDbl(std::count(selection.cbegin(), selection.cend(), 2)) * 100.0 / Count;
        const auto percent1 = toDbl(std::count(selection.cbegin(), selection.cend(), 1)) * 100.0 / Count;
        ASSERT_EQ(selection.size(), Count);
        ASSERT_NEAR(percent1 + percent2 + percent5 + percent10, 100, 0.1);

        std::cout << "Distribution:" << std::endl;
        std::cout << "10s: " << percent10 << "%" << std::endl;
        std::cout << "5s: " << percent5 << "%" << std::endl;
        std::cout << "2s: " << percent2 << "%" << std::endl;
        std::cout << "1s: " << percent1 << "%" << std::endl;

        EXPECT_NEAR(percent10, 55, 1);
        EXPECT_NEAR(percent5, 27, 1);
        EXPECT_NEAR(percent2, 11, 1);
        EXPECT_NEAR(percent1, 5, 1);
    }

    { // General implementation
        const auto selection = utils_cpp::random_weighted_items(items, Count, {}, 0);
        const auto percent10 = toDbl(std::count(selection.cbegin(), selection.cend(), 10)) * 100.0 / Count;
        const auto percent5 = toDbl(std::count(selection.cbegin(), selection.cend(), 5)) * 100.0 / Count;
        const auto percent2 = toDbl(std::count(selection.cbegin(), selection.cend(), 2)) * 100.0 / Count;
        const auto percent1 = toDbl(std::count(selection.cbegin(), selection.cend(), 1)) * 100.0 / Count;
        ASSERT_EQ(selection.size(), Count);
        ASSERT_NEAR(percent1 + percent2 + percent5 + percent10, 100, 0.1);

        std::cout << "Distribution:" << std::endl;
        std::cout << "10s: " << percent10 << "%" << std::endl;
        std::cout << "5s: " << percent5 << "%" << std::endl;
        std::cout << "2s: " << percent2 << "%" << std::endl;
        std::cout << "1s: " << percent1 << "%" << std::endl;

        EXPECT_NEAR(percent10, 55, 1);
        EXPECT_NEAR(percent5, 27, 1);
        EXPECT_NEAR(percent2, 11, 1);
        EXPECT_NEAR(percent1, 5, 1);
    }
}

TEST(utils_cpp, ContainerUtilsTest_random_weighted_item_stats2)
{
    constexpr size_t Count = 100000; // 100k
    const std::vector<int> items {10, 5, 2, 1};

    const auto selection = utils_cpp::generate<std::vector>(Count, [&items](){ return utils_cpp::random_weighted_item(items); });
    const auto percent10 = toDbl(std::count(selection.cbegin(), selection.cend(), 10)) * 100.0 / Count;
    const auto percent5 = toDbl(std::count(selection.cbegin(), selection.cend(), 5)) * 100.0 / Count;
    const auto percent2 = toDbl(std::count(selection.cbegin(), selection.cend(), 2)) * 100.0 / Count;
    const auto percent1 = toDbl(std::count(selection.cbegin(), selection.cend(), 1)) * 100.0 / Count;
    ASSERT_EQ(selection.size(), Count);
    ASSERT_NEAR(percent1 + percent2 + percent5 + percent10, 100, 0.1);

    std::cout << "Distribution:" << std::endl;
    std::cout << "10s: " << percent10 << "%" << std::endl;
    std::cout << "5s: " << percent5 << "%" << std::endl;
    std::cout << "2s: " << percent2 << "%" << std::endl;
    std::cout << "1s: " << percent1 << "%" << std::endl;

    EXPECT_NEAR(percent10, 55, 1.25);
    EXPECT_NEAR(percent5, 27, 1.25);
    EXPECT_NEAR(percent2, 11, 1.25);
    EXPECT_NEAR(percent1, 5, 1.25);
}

TEST(utils_cpp, ContainerUtilsTest_random_weighted_items)
{
    const std::vector<int> items {1, 2, 3, 4, 5, 6, 7, 8, 9, 1000};
    std::vector<int> selection = utils_cpp::random_weighted_items(items, 10);
    const auto cnt1000 = std::count(selection.cbegin(), selection.cend(), 1000);
    EXPECT_GE(cnt1000, 7);

    const std::set<int> setSelection (selection.cbegin(), selection.cend());
    EXPECT_LT(setSelection.size(), selection.size());
}

TEST(utils_cpp, ContainerUtilsTest_random_weighted_item_unique)
{
    const std::vector<int> items {1, 2, 3, 4, 5, 6, 7, 8, 9, 1000};
    std::vector<int> selection = utils_cpp::random_weighted_items_unique(items, 10);
    EXPECT_TRUE(selection[0] == 1000 || selection[1] == 1000);

    const std::set<int> setSelection (selection.cbegin(), selection.cend());
    EXPECT_EQ(setSelection.size(), selection.size());
}

TEST(utils_cpp, ContainerUtilsTest_random_weighted_items_coverage)
{
    constexpr size_t Count = 30;
    const std::vector<int> items {50, 51, 52};
    const auto selection1 = utils_cpp::generate<std::vector>(Count, [&items](){ return utils_cpp::random_weighted_item(items); });
    const auto selection2 = utils_cpp::random_weighted_items(items, Count);
    const auto selection3 = utils_cpp::random_weighted_items_unique(items, 3);
    EXPECT_TRUE(utils_cpp::contains(selection1, 50));
    EXPECT_TRUE(utils_cpp::contains(selection1, 51));
    EXPECT_TRUE(utils_cpp::contains(selection1, 52));

    EXPECT_TRUE(utils_cpp::contains(selection2, 50));
    EXPECT_TRUE(utils_cpp::contains(selection2, 51));
    EXPECT_TRUE(utils_cpp::contains(selection2, 52));

    EXPECT_TRUE(utils_cpp::contains(selection3, 50));
    EXPECT_TRUE(utils_cpp::contains(selection3, 51));
    EXPECT_TRUE(utils_cpp::contains(selection3, 52));
}

TEST(utils_cpp, ContainerUtilsTest_random_weighted_item_all)
{
    std::vector<int> items = {10, 5, 2, 1};
    size_t count = items.size();

    auto selection = utils_cpp::random_weighted_items_unique(items, count);

    EXPECT_EQ(selection.size(), count);

    std::set<int> selectionSet(selection.begin(), selection.end());
    std::set<int> itemsSet(items.begin(), items.end());
    EXPECT_EQ(selectionSet, itemsSet);
}

TEST(utils_cpp, ContainerUtilsTest_random_weighted_item_subset)
{
    std::vector<int> items = {10, 5, 2, 1};
    size_t count = 2;

    const int iterations = 100000; // 100k
    std::map<int, int> frequency;

    for (int i = 0; i < iterations; i++) {
        auto selection = utils_cpp::random_weighted_items_unique(items, count);

        EXPECT_EQ(selection.size(), count);

        std::set<int> uniqueItems(selection.begin(), selection.end());
        EXPECT_EQ(uniqueItems.size(), count);

        for (auto value : selection)
            frequency[value]++;
    }

    double percent10 = frequency[10] * 100.0 / iterations;
    double percent5  = frequency[5]  * 100.0 / iterations;
    double percent2  = frequency[2]  * 100.0 / iterations;
    double percent1  = frequency[1]  * 100.0 / iterations;

    std::cout << "Frequency distribution for subset selection (m=2):\n";
    std::cout << "10: " << percent10 << "%" << std::endl;
    std::cout << "5: "  << percent5  << "%" << std::endl;
    std::cout << "2: "  << percent2  << "%" << std::endl;
    std::cout << "1: "  << percent1  << "%" << std::endl;

    EXPECT_NEAR(percent10, 86.42, 2.0);
    EXPECT_NEAR(percent5,  67.13, 2.0);
    EXPECT_NEAR(percent2,  30.79, 2.0);
    EXPECT_NEAR(percent1,  15.66, 2.0);
}

TEST(utils_cpp, ContainerUtilsTest_random_weighted_item_single)
{
    std::vector<int> items = {10, 5, 2, 1};
    size_t count = 1;

    const int iterations = 100000; // 100k
    std::map<int, int> frequency;

    for (int i = 0; i < iterations; i++) {
        auto selection = utils_cpp::random_weighted_items_unique(items, count);

        EXPECT_EQ(selection.size(), count);

        frequency[selection[0]]++;
    }

    double percent10 = frequency[10] * 100.0 / iterations;
    double percent5  = frequency[5]  * 100.0 / iterations;
    double percent2  = frequency[2]  * 100.0 / iterations;
    double percent1  = frequency[1]  * 100.0 / iterations;

    std::cout << "Distribution for single item selection:\n";
    std::cout << "10: " << percent10 << "%" << std::endl;
    std::cout << "5: "  << percent5  << "%" << std::endl;
    std::cout << "2: "  << percent2  << "%" << std::endl;
    std::cout << "1: "  << percent1  << "%" << std::endl;

    EXPECT_NEAR(percent10, 55.88, 2.0);
    EXPECT_NEAR(percent5,  28.38, 2.0);
    EXPECT_NEAR(percent2,  10.45, 2.0);
    EXPECT_NEAR(percent1,   5.29, 2.0);
}

TEST(utils_cpp, ContainerUtilsTest_random_weighted_item_zero)
{
    std::vector<int> items = {10, 5, 2, 1};
    auto selection = utils_cpp::random_weighted_items_unique(items, 0);
    EXPECT_TRUE(selection.empty());
}
