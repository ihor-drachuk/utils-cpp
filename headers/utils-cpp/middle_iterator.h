#pragma once
#include <iterator>
#include <tuple>
#include <cmath>
#include <cassert>

template<typename Iter>
class middle_iterator
{
public:
    using iterator_category = std::random_access_iterator_tag;
    using difference_type = int;
    using value_type = typename Iter::value_type;
    using pointer = typename Iter::pointer;
    using reference = typename Iter::reference;

    middle_iterator(Iter begin, Iter end)
        : m_begin(begin),
          m_end(end),
          m_total(std::distance(begin, end)),
          m_index(0)
    {
        calculateOffset();
    }

    middle_iterator(Iter begin, Iter end, size_t index)
        : m_begin(begin),
          m_end(end),
          m_total(std::distance(begin, end)),
          m_index(index)
    {
        calculateOffset();
    }

    reference operator*() { assert(isValid()); return *(m_begin + m_offset); }
    pointer operator->() { assert(isValid()); return &**this; }

    bool operator== (const middle_iterator<Iter>& rhs) const { return m_index == rhs.m_index; }
    bool operator!= (const middle_iterator<Iter>& rhs) const { return m_index != rhs.m_index; }
    bool operator<  (const middle_iterator<Iter>& rhs) const { return m_index <  rhs.m_index; }
    bool operator<= (const middle_iterator<Iter>& rhs) const { return m_index <= rhs.m_index; }
    bool operator>  (const middle_iterator<Iter>& rhs) const { return m_index >  rhs.m_index; }
    bool operator>= (const middle_iterator<Iter>& rhs) const { return m_index >= rhs.m_index; }

    middle_iterator<Iter>& operator++() { advance(1); return *this; }
    middle_iterator<Iter>  operator++(int) { middle_iterator<Iter> n = *this; advance(1); return n; }
    middle_iterator<Iter>& operator--() { advance(-1); return *this; }
    middle_iterator<Iter>  operator--(int) { middle_iterator<Iter> n = *this; advance(-1); return n; }
    middle_iterator<Iter>& operator+=(int i) { advance(i); return *this; }
    middle_iterator<Iter>& operator-=(int i) { advance(-i); return *this; }
    middle_iterator<Iter>  operator+(int i) const { return middle_iterator<Iter>(m_begin, m_end, m_offset + i); }
    middle_iterator<Iter>  operator-(int i) const { return middle_iterator<Iter>(m_begin, m_end, m_offset - i); }
    difference_type operator-(const middle_iterator<Iter>& rhs) const { return m_index - rhs.m_index; }

    bool isValid() const { return m_index < m_total; };

private:
    void calculateOffset() {
        auto sign = lround(pow(-1, m_index + 2));
        m_offset = m_total / 2 + (m_index + 1) / 2 * sign;
    }
    void advance(int i) {
        m_index += i;
        calculateOffset();
    }

private:
    Iter m_begin, m_end;
    size_t m_total {};
    size_t m_index {};
    size_t m_offset {};
};
