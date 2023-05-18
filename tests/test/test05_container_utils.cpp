#include <gtest/gtest.h>
#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <list>
#include <utils-cpp/container_utils.h>
#include <utils-cpp/comparison_traits.h>


TEST(utils_cpp, ContainerUtilsTest)
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

    ASSERT_TRUE(utils_cpp::find(list, 1).has_value());
    ASSERT_TRUE(utils_cpp::find(list, 2).has_value());
    ASSERT_EQ(utils_cpp::find(list, 2).index(), 1);
    ASSERT_TRUE(utils_cpp::find(list, 3).has_value());
    ASSERT_FALSE(utils_cpp::find(list, 4).has_value());

    ASSERT_TRUE(utils_cpp::find_in_set(set, 1).has_value());
    ASSERT_TRUE(utils_cpp::find_in_set(set, 2).has_value());
    ASSERT_TRUE(utils_cpp::find_in_set(set, 3).has_value());
    ASSERT_FALSE(utils_cpp::find_in_set(set, 4).has_value());
    ASSERT_TRUE(utils_cpp::find_in_set(unordered_set, 1).has_value());
    ASSERT_FALSE(utils_cpp::find_in_set(unordered_set, 4).has_value());

    ASSERT_TRUE(utils_cpp::find_in_map(map, 1).has_value());
    ASSERT_TRUE(utils_cpp::find_in_map(map, 2).has_value());
    ASSERT_TRUE(utils_cpp::find_in_map(map, 3).has_value());
    ASSERT_FALSE(utils_cpp::find_in_map(map, 4).has_value());
    ASSERT_TRUE(utils_cpp::find_in_map(unordered_map, 1).has_value());
    ASSERT_FALSE(utils_cpp::find_in_map(unordered_map, 4).has_value());
}
