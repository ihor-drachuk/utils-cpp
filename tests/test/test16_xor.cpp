/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/utils-qt
 * Contact:  ihor-drachuk-libs@pm.me  */

#include <gtest/gtest.h>
#include <utils-cpp/xor.h>
#include <utils-cpp/functor_iterator.h>
#include "internal/data_10kb.h"

TEST(utils_cpp, test_xor)
{
    // Approach 1
    auto buffer1 = data_10kb_1();
    const auto mask = data_10kb_2();

    for (size_t i = 0; i < buffer1.size(); i++)
        utils_cpp::xorByte(buffer1[i], mask[i]);

    // Approach 2
    auto buffer2 = data_10kb_1();
    utils_cpp::xorBuffer(buffer2.data(), mask.data(), buffer2.size());

    // Approach 3
    auto buffer3 = data_10kb_1();
    utils_cpp::xorBufferIt(buffer3.begin(), buffer3.end(), mask.begin());

    // Approach 4
    auto buffer4 = data_10kb_1();
    auto functor = [it = mask.begin()]() mutable { return *it++; };
    utils_cpp::xorBufferIt(buffer4.begin(), buffer4.end(), functor_iterator(functor));

    EXPECT_EQ(buffer1, buffer2);
    EXPECT_EQ(buffer1, buffer3);
    EXPECT_EQ(buffer1, buffer4);
}

TEST(utils_cpp, test_xor_empty)
{
    std::vector<uint8_t> buffer1;
    const std::vector<uint8_t> mask;

    utils_cpp::xorBuffer(buffer1.data(), mask.data(), buffer1.size());
    utils_cpp::xorBufferIt(buffer1.begin(), buffer1.end(), mask.begin());
}
