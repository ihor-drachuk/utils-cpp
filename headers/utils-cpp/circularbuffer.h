#pragma once
#include <cstddef>

class CircularBuffer
{
public:
    CircularBuffer(size_t capacity);
    ~CircularBuffer();

    size_t size() const { return m_size; }
    size_t capacity() const { return m_capacity; }
    size_t write(const void* data, size_t bytes);
    size_t fill(unsigned char byte, size_t size);
    size_t read(void* data, size_t bytes);
    void reset();

private:
    template<typename A, typename B>
    inline static A min(A a, B b) {
        return (a < b) ? a : b;
    }

private:
    size_t m_begIndex, m_endIndex, m_size, m_capacity;
    unsigned char* m_data;
};
