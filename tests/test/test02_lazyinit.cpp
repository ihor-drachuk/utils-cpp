/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/utils-cpp
 * Contact:  ihor-drachuk-libs@pm.me  */

#include <gtest/gtest.h>
#include <utils-cpp/lazyinit.h>

static bool testFlag = false;

struct MyObj
{
    static MyObj* create() { return new MyObj(); }
    MyObj() { testFlag = true; }
};

TEST(utils_cpp, LazyInitTest)
{
    LazyInit<MyObj> obj([]()->auto { return new MyObj(); });
    ASSERT_FALSE(testFlag);
    ASSERT_FALSE(obj);
    *obj;
    ASSERT_TRUE(testFlag);
    ASSERT_TRUE(obj);

    LazyInit<MyObj> obj2(&MyObj::create);
}
