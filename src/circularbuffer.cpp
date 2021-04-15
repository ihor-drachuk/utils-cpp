#include "circularbuffer.h"
#include <cstring>


CircularBuffer::CircularBuffer(size_t capacity)
    : m_capacity(capacity)
{
    m_data = new unsigned char[capacity];
    reset();
}

CircularBuffer::~CircularBuffer()
{
    delete[] m_data;
}

size_t CircularBuffer::write(const void *d, size_t bytes)
{
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

size_t CircularBuffer::read(void *d, size_t bytes)
{
    if (bytes == 0) return 0;

    unsigned char* data = (unsigned char*)d;
    size_t capacity = m_capacity;
    size_t bytes_to_read = min(bytes, m_size);

    // Read in a single step
    if (bytes_to_read <= capacity - m_begIndex)
    {
        memcpy(data, m_data + m_begIndex, bytes_to_read);
        m_begIndex += bytes_to_read;
        if (m_begIndex == capacity) m_begIndex = 0;
    }
    // Read in two steps
    else
    {
        size_t size_1 = capacity - m_begIndex;
        memcpy(data, m_data + m_begIndex, size_1);
        size_t size_2 = bytes_to_read - size_1;
        memcpy(data + size_1, m_data, size_2);
        m_begIndex = size_2;
    }

    m_size -= bytes_to_read;
    return bytes_to_read;
}

void CircularBuffer::reset()
{
    m_begIndex = 0;
    m_endIndex = 0;
    m_size = 0;
}
