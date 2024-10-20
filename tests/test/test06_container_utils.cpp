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

CREATE_CHECK_METHOD(index)


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
