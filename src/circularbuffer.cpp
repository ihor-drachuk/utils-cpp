#include "utils-cpp/circularbuffer.h"
#include <cstring>
#include <cassert>


CircularBuffer::CircularBuffer(size_t capacity)
    : m_capacity(capacity)
{
    m_data = new unsigned char[capacity];
    reset();
}

CircularBuffer::CircularBuffer(const CircularBuffer& rhs)
    : CircularBuffer(rhs.capacity())
{
    rhs.readRO(m_data, rhs.m_size);
    m_endIndex = rhs.m_size;
    m_size = rhs.m_size;
}

CircularBuffer::CircularBuffer(CircularBuffer&& rhs)
{
    assert(!rhs.m_moved);
    this->tie() = rhs.tie();
    rhs.m_moved = true;
}

CircularBuffer::~CircularBuffer()
{
    if (m_moved)
        return;

    delete[] m_data;
}

CircularBuffer& CircularBuffer::operator=(const CircularBuffer& rhs)
{
    assert(!rhs.m_moved);

    if (this == &rhs)
        return *this;

    if (m_capacity != rhs.m_capacity) {
        auto buffer = new unsigned char[rhs.m_capacity];

        delete[] m_data;
        m_data = buffer;
        m_capacity = rhs.m_capacity;
    }

    rhs.readRO(m_data, rhs.m_size);
    m_begIndex = 0;
    m_endIndex = rhs.m_size;
    m_size = rhs.m_size;

    return *this;
}

CircularBuffer& CircularBuffer::operator=(CircularBuffer&& rhs)
{
    assert(!rhs.m_moved);

    if (this == &rhs)
        return *this;

    delete[] m_data;
    this->tie() = rhs.tie();
    rhs.m_moved = true;

    return *this;
}

size_t CircularBuffer::write(const void* d, size_t bytes)
{
    assert(!m_moved);
    if (bytes == 0) return 0;

    const unsigned char* data = (const unsigned char*)d;
    size_t capacity = m_capacity;
    size_t bytes_to_write = min(bytes, capacity - m_size);

    // Write in a single step
    if (bytes_to_write <= capacity - m_endIndex)
    {
        memcpy(m_data + m_endIndex, data, bytes_to_write);
        m_endIndex += bytes_to_write;
        if (m_endIndex == capacity) m_endIndex = 0;
    }
    // Write in two steps
    else
    {
        size_t size_1 = capacity - m_endIndex;
        memcpy(m_data + m_endIndex, data, size_1);
        size_t size_2 = bytes_to_write - size_1;
        memcpy(m_data, data + size_1, size_2);
        m_endIndex = size_2;
    }

    m_size += bytes_to_write;
    return bytes_to_write;
}

size_t CircularBuffer::fill(unsigned char byte, size_t size)
{
    assert(!m_moved);
    if (size == 0) return 0;

    size_t capacity = m_capacity;
    size_t bytes_to_write = min(size, capacity - m_size);

    // Write in a single step
    if (bytes_to_write <= capacity - m_endIndex)
    {
        memset(m_data + m_endIndex, byte, bytes_to_write);
        m_endIndex += bytes_to_write;
        if (m_endIndex == capacity) m_endIndex = 0;
    }
    // Write in two steps
    else
    {
        size_t size_1 = capacity - m_endIndex;
        memset(m_data + m_endIndex, byte, size_1);
        size_t size_2 = bytes_to_write - size_1;
        memset(m_data, byte, size_2);
        m_endIndex = size_2;
    }

    m_size += bytes_to_write;
    return bytes_to_write;
}

size_t CircularBuffer::read(void* d, size_t bytes, bool erase)
{
    assert(!m_moved);
    if (bytes == 0) return 0;

    unsigned char* data = (unsigned char*)d;
    size_t capacity = m_capacity;
    size_t bytes_to_read = min(bytes, m_size);

    size_t temp_size = m_size;
    size_t temp_begIndex = m_begIndex;
    size_t& size = erase ? m_size : temp_size;
    size_t& begIndex = erase ? m_begIndex : temp_begIndex;

    // Read in a single step
    if (bytes_to_read <= capacity - begIndex)
    {
        memcpy(data, m_data + begIndex, bytes_to_read);
        begIndex += bytes_to_read;
        if (begIndex == capacity) begIndex = 0;
    }
    // Read in two steps
    else
    {
        size_t size_1 = capacity - begIndex;
        memcpy(data, m_data + begIndex, size_1);
        size_t size_2 = bytes_to_read - size_1;
        memcpy(data + size_1, m_data, size_2);
        begIndex = size_2;
    }

    size -= bytes_to_read;
    return bytes_to_read;
}

size_t CircularBuffer::readRO(void* data, size_t bytes) const
{
    assert(!m_moved);
    return const_cast<CircularBuffer*>(this)->read(data, bytes, false);
}

void CircularBuffer::reset()
{
    assert(!m_moved);
    m_begIndex = 0;
    m_endIndex = 0;
    m_size = 0;
}
