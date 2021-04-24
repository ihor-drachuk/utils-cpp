#include "gtest/gtest.h"

static int add(int a, int b) {
    return a+b;
}

TEST(add, test00)
{
    ASSERT_EQ(add(10, 20), 30);
}
