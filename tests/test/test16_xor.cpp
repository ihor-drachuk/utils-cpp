/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/utils-qt
 * Contact:  ihor-drachuk-libs@pm.me  */

#include <gtest/gtest.h>
#include <utils-cpp/xor.h>
#include <utils-cpp/functor_iterator.h>
#include "internal/data_10kb.h"
#include <cstring>
#include <numeric>

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

namespace {

// Helper: compute XOR reference result byte-by-byte
std::vector<uint8_t> xorReference(const std::vector<uint8_t>& dst, const std::vector<uint8_t>& src)
{
    std::vector<uint8_t> result = dst;
    for (size_t i = 0; i < result.size(); ++i)
        result[i] ^= src[i];
    return result;
}

// Helper: create a buffer with a deliberate misalignment offset
struct UnalignedBuffer
{
    std::vector<uint8_t> storage;
    uint8_t* ptr;

    UnalignedBuffer(const std::vector<uint8_t>& data, size_t offset)
        : storage(data.size() + alignof(uint64_t) + offset)
    {
        // Find an aligned address, then shift by offset to guarantee misalignment
        auto* base = storage.data();
        auto addr = reinterpret_cast<uintptr_t>(base);
        auto aligned = (addr + alignof(uint64_t) - 1) & ~(alignof(uint64_t) - 1);
        ptr = reinterpret_cast<uint8_t*>(aligned) + offset;
        std::memcpy(ptr, data.data(), data.size());
    }
};

void testXorUnaligned(size_t sz, size_t dstOffset, size_t srcOffset)
{
    std::vector<uint8_t> dstData(sz);
    std::vector<uint8_t> srcData(sz);
    std::iota(dstData.begin(), dstData.end(), uint8_t(1));
    std::iota(srcData.begin(), srcData.end(), uint8_t(137));

    const auto expected = xorReference(dstData, srcData);

    UnalignedBuffer dst(dstData, dstOffset);
    UnalignedBuffer src(srcData, srcOffset);

    utils_cpp::xorBuffer(dst.ptr, src.ptr, sz);

    EXPECT_EQ(0, std::memcmp(dst.ptr, expected.data(), sz))
        << "Failed for sz=" << sz << " dstOffset=" << dstOffset << " srcOffset=" << srcOffset;
}

} // namespace

// Aligned pointers (fast path)
TEST(utils_cpp, test_xor_aligned)
{
    testXorUnaligned(64, 0, 0);
    testXorUnaligned(256, 0, 0);
    testXorUnaligned(1024, 0, 0);
}

// Unaligned, small buffers (stack path)
TEST(utils_cpp, test_xor_unaligned_small)
{
    for (size_t offset = 1; offset < alignof(uint64_t); ++offset) {
        testXorUnaligned(1, offset, 0);
        testXorUnaligned(7, 0, offset);
        testXorUnaligned(16, offset, offset);
        testXorUnaligned(32, offset, 0);
        testXorUnaligned(32, 0, offset);
    }
}

// Unaligned, large buffers (heap path)
TEST(utils_cpp, test_xor_unaligned_large)
{
    for (size_t offset = 1; offset < alignof(uint64_t); ++offset) {
        testXorUnaligned(256, offset, 0);
        testXorUnaligned(256, 0, offset);
        testXorUnaligned(1024, offset, offset);
        testXorUnaligned(4096, offset, 0);
    }
}

// Edge: size not multiple of 8 (tail handling)
TEST(utils_cpp, test_xor_tail_handling)
{
    for (size_t sz = 1; sz <= 17; ++sz) {
        testXorUnaligned(sz, 0, 0);   // aligned
        testXorUnaligned(sz, 1, 1);   // unaligned
    }
}
