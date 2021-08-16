#include <gtest/gtest.h>
#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <list>
#include <utils-cpp/container_utils.h>


TEST(UtilsCpp, ContainerUtilsTest)
{
    std::vector<int> vector {1, 2, 3};
    std::list<int> list {1, 2, 3};

    std::set<int> set {1, 2, 3};
    std::unordered_set<int> unordered_set {1, 2, 3};

    std::map<int, std::string> map {{1, "one"}, {2, "two"}, {3, "three"}};
    std::unordered_map<int, std::string> unordered_map {{1, "one"}, {2, "two"}, {3, "three"}};

    ASSERT_TRUE(UtilsCpp::find(vector, 1).has_value());
    ASSERT_TRUE(UtilsCpp::find(vector, 2).has_value());
    ASSERT_TRUE(UtilsCpp::find(vector, 3).has_value());
    ASSERT_FALSE(UtilsCpp::find(vector, 4).has_value());

    ASSERT_TRUE(UtilsCpp::find(list, 1).has_value());
    ASSERT_TRUE(UtilsCpp::find(list, 2).has_value());
    ASSERT_TRUE(UtilsCpp::find(list, 3).has_value());
    ASSERT_FALSE(UtilsCpp::find(list, 4).has_value());

    ASSERT_TRUE(UtilsCpp::find_in_set(set, 1).has_value());
    ASSERT_TRUE(UtilsCpp::find_in_set(set, 2).has_value());
    ASSERT_TRUE(UtilsCpp::find_in_set(set, 3).has_value());
    ASSERT_FALSE(UtilsCpp::find_in_set(set, 4).has_value());
    ASSERT_TRUE(UtilsCpp::find_in_set(unordered_set, 1).has_value());
    ASSERT_FALSE(UtilsCpp::find_in_set(unordered_set, 4).has_value());

    ASSERT_TRUE(UtilsCpp::find_in_map(map, 1).has_value());
    ASSERT_TRUE(UtilsCpp::find_in_map(map, 2).has_value());
    ASSERT_TRUE(UtilsCpp::find_in_map(map, 3).has_value());
    ASSERT_FALSE(UtilsCpp::find_in_map(map, 4).has_value());
    ASSERT_TRUE(UtilsCpp::find_in_map(unordered_map, 1).has_value());
    ASSERT_FALSE(UtilsCpp::find_in_map(unordered_map, 4).has_value());
}
