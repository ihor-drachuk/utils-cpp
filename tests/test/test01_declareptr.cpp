#include <gtest/gtest.h>
#include <utils-cpp/declareptr.h>
#include <memory>

class TestClass { };
DECLARE_PTR(TestClass);

struct TestStruct { };
DECLARE_PTR_STRUCT(TestStruct);

TEST(utils_cpp, DeclarePtrTest)
{
    TestClassPtr a;
    TestStructPtr b;
    a.reset();
    b.reset();
}
