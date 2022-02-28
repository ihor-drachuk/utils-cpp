#pragma once
#include <cstddef>
#include <tuple>

class CircularBuffer
{
public:
    CircularBuffer(size_t capacity);
    CircularBuffer(const CircularBuffer& rhs);
    CircularBuffer(CircularBuffer&& rhs);
    ~CircularBuffer();

    CircularBuffer& operator=(const CircularBuffer& rhs);
    CircularBuffer& operator=(CircularBuffer&& rhs);

    size_t size() const { return m_size; }
    size_t capacity() const { return m_capacity; }
    size_t write(const void* data, size_t bytes);
    size_t fill(unsigned char byte, size_t size);
    size_t read(void* data, size_t bytes, bool erase = true);
    size_t readRO(void* data, size_t bytes) const;
    void reset();

private:
    template<typename A, typename B>
    inline static A min(A a, B b) {
        return (a < b) ? a : b;
    }

    auto tie() { return std::tie(m_begIndex, m_endIndex, m_size, m_capacity, m_data, m_moved); }

private:
    size_t m_begIndex, m_endIndex, m_size, m_capacity;
    unsigned char* m_data;
    bool m_moved { false };
};
