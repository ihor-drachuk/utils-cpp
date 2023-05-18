#include <gtest/gtest.h>
#include <utils-cpp/comparison_traits.h>

using namespace utils_cpp;

namespace {
struct Unknown;

struct Int {
    operator int ();
};

struct Comp {
    bool operator==(const Comp&) const;
    bool operator>(const Comp&) const;
};
} // namespace

TEST(utils_cpp, ComparisonTraits)
{
    ASSERT_TRUE((is_comparable_eq<int, int>::value));
    ASSERT_TRUE((is_comparable_eq<int, Int>::value));
    ASSERT_FALSE((is_comparable_eq<int, std::string>::value));
    ASSERT_FALSE((is_comparable_eq<int, Unknown>::value));

    ASSERT_TRUE ((is_comparable_eq<Comp, Comp>::value));
    ASSERT_FALSE((is_comparable_ne<Comp, Comp>::value));
    ASSERT_TRUE ((is_comparable_gt<Comp, Comp>::value));
    ASSERT_FALSE((is_comparable_ge<Comp, Comp>::value));
    ASSERT_FALSE((is_comparable_lt<Comp, Comp>::value));
    ASSERT_FALSE((is_comparable_le<Comp, Comp>::value));
}
