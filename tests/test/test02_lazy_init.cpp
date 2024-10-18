/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/utils-cpp
 * Contact:  ihor-drachuk-libs@pm.me  */

#include <gtest/gtest.h>
#include <utils-cpp/lazy_init.h>

static bool testFlag = false;

struct MyObj
{
    static MyObj* create() { return new MyObj(); }
    MyObj() { testFlag = true; }
};

TEST(utils_cpp, lazy_init_test)
{
    utils_cpp::lazy_init<MyObj> obj;
    ASSERT_FALSE(testFlag);
    ASSERT_FALSE(obj);
    *obj;
    ASSERT_TRUE(testFlag);
    ASSERT_TRUE(obj);

    testFlag = false;
    utils_cpp::lazy_init_custom<MyObj> obj2(&MyObj::create);
    ASSERT_FALSE(testFlag);
    ASSERT_FALSE(obj2);
    *obj2;
    ASSERT_TRUE(testFlag);
    ASSERT_TRUE(obj2);
}
