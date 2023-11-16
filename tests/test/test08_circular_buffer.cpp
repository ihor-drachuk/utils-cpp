/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/utils-cpp
 * Contact:  ihor-drachuk-libs@pm.me  */

#include <gtest/gtest.h>
#include <utils-cpp/circularbuffer.h>


namespace {

size_t constexpr length(const char* str)
{
    return *str ? 1 + length(str + 1) : 0;
}

constexpr auto phrase = "Hello";
constexpr auto phraseLen = length(phrase);
constexpr uint8_t mark = 17;

} // namespace


TEST(utils_cpp, CircularBuffer_RO_RW)
{
    CircularBuffer buf(phraseLen);
    char tempBase[phraseLen+2] = {0};
    char* temp = tempBase+1;
    temp[-1] = mark;
    temp[phraseLen] = mark;

    for (int i = 0; i < 2; i++) {
        ASSERT_EQ(buf.size(), 0);

        auto r = buf.write(phrase, phraseLen);
        ASSERT_EQ(r, phraseLen);

        memset(temp, 0, phraseLen);
        r = buf.readRO(temp, phraseLen);
        ASSERT_EQ(r, phraseLen);
        ASSERT_EQ(temp[-1], mark);
        ASSERT_EQ(temp[phraseLen], mark);
        ASSERT_EQ(memcmp(temp, phrase, phraseLen), 0);

        memset(temp, 0, phraseLen);
        r = buf.readRO(temp, phraseLen);
        ASSERT_EQ(r, phraseLen);
        ASSERT_EQ(temp[-1], mark);
        ASSERT_EQ(temp[phraseLen], mark);
        ASSERT_EQ(memcmp(temp, phrase, phraseLen), 0);

        memset(temp, 0, phraseLen);
        r = buf.read(temp, phraseLen, false);
        ASSERT_EQ(r, phraseLen);
        ASSERT_EQ(temp[-1], mark);
        ASSERT_EQ(temp[phraseLen], mark);
        ASSERT_EQ(memcmp(temp, phrase, phraseLen), 0);

        memset(temp, 0, phraseLen);
        r = buf.read(temp, phraseLen, false);
        ASSERT_EQ(r, phraseLen);
        ASSERT_EQ(temp[-1], mark);
        ASSERT_EQ(temp[phraseLen], mark);
        ASSERT_EQ(memcmp(temp, phrase, phraseLen), 0);

        memset(temp, 0, phraseLen);
        r = buf.read(temp, phraseLen);
        ASSERT_EQ(r, phraseLen);
        ASSERT_EQ(temp[-1], mark);
        ASSERT_EQ(temp[phraseLen], mark);
        ASSERT_EQ(memcmp(temp, phrase, phraseLen), 0);

        //memset(temp, 0, phraseLen);
        r = buf.read(temp, phraseLen);
        ASSERT_EQ(r, 0);
        ASSERT_EQ(temp[-1], mark);
        ASSERT_EQ(temp[phraseLen], mark);
        ASSERT_EQ(memcmp(temp, phrase, phraseLen), 0);
    }
}

TEST(utils_cpp, CircularBuffer_Copy_Move)
{
    CircularBuffer buf(phraseLen);
    buf.write(phrase, phraseLen);

    char data[phraseLen];

    CircularBuffer buf2(buf);
    ASSERT_EQ(buf.capacity(), phraseLen);
    ASSERT_EQ(buf.size(), phraseLen);
    ASSERT_EQ(buf2.capacity(), phraseLen);
    ASSERT_EQ(buf2.size(), phraseLen);
    buf2.readRO(data, phraseLen);
    ASSERT_EQ(memcmp(data, phrase, phraseLen), 0);

    memset(data, 0, phraseLen);
    CircularBuffer buf3(1);
    buf3 = buf;
    ASSERT_EQ(buf.capacity(), phraseLen);
    ASSERT_EQ(buf.size(), phraseLen);
    ASSERT_EQ(buf3.capacity(), phraseLen);
    ASSERT_EQ(buf3.size(), phraseLen);
    buf3.readRO(data, phraseLen);
    ASSERT_EQ(memcmp(data, phrase, phraseLen), 0);

    memset(data, 0, phraseLen);
    CircularBuffer buf4(std::move(buf));
    ASSERT_EQ(buf4.capacity(), phraseLen);
    ASSERT_EQ(buf4.size(), phraseLen);
    buf4.readRO(data, phraseLen);
    ASSERT_EQ(memcmp(data, phrase, phraseLen), 0);

    memset(data, 0, phraseLen);
    CircularBuffer buf5(1);
    buf5 = std::move(buf4);
    ASSERT_EQ(buf5.capacity(), phraseLen);
    ASSERT_EQ(buf5.size(), phraseLen);
    buf5.readRO(data, phraseLen);
    ASSERT_EQ(memcmp(data, phrase, phraseLen), 0);
}
