/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/utils-qt
 * Contact:  ihor-drachuk-libs@pm.me  */

#include <gtest/gtest.h>
#include <algorithm>
#include <utils-cpp/functor_iterator.h>

TEST(utils_cpp, functor_iterator)
{
    std::vector<int> vec;

    auto functor = [i = 0]() mutable { return ++i; };

    std::copy(functor_iterator(functor),
              functor_iterator(functor, 5),
              std::back_inserter(vec));

    ASSERT_EQ(vec, (std::vector<int>{1, 2, 3, 4, 5}));
}

TEST(utils_cpp, functor_iterator_empty)
{
    std::vector<int> vec;

    auto functor = [i = 0]() mutable { return ++i; };

    std::copy(functor_iterator(functor),
              functor_iterator(functor, 0),
              std::back_inserter(vec));

    ASSERT_EQ(vec, (std::vector<int>()));
}
