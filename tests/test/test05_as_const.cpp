#include <gtest/gtest.h>
#include <utils-cpp/as_const.h>

using namespace utils_cpp;

namespace {
struct SomeStruct {
    int value() { return 1; }
    int value() const { return 2; }
};
} // namespace

TEST(utils_cpp, AsConstTest)
{
    SomeStruct s;
    ASSERT_EQ(s.value(), 1);
    ASSERT_EQ(as_const(s).value(), 2);
}
