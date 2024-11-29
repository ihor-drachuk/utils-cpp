/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/utils-cpp
 * Contact:  ihor-drachuk-libs@pm.me  */

#pragma once
#include <iterator>
#include <type_traits>

template<typename Func,
         typename T = std::invoke_result_t<Func>>
class functor_iterator
{
public:
    using This = functor_iterator<Func, T>;
    using iterator_category = std::forward_iterator_tag;
    using difference_type = int;
    using value_type = T;
    using pointer = T*;
    using reference = T&;

    functor_iterator(Func functor)
        : m_functor(functor)
    {
    }

    functor_iterator(Func functor, size_t position)
        : m_functor(functor),
          m_position(position)
    {
    }

    functor_iterator(const functor_iterator&) = default;
    functor_iterator& operator=(const functor_iterator&) = default;

    value_type operator*() { return m_functor(); }
    //reference operator*() { return m_functor(); }
    //pointer operator->() { &**this; }

    bool operator== (const This& rhs) const { return m_position == rhs.m_position; }
    bool operator!= (const This& rhs) const { return m_position != rhs.m_position; }
    bool operator<  (const This& rhs) const { return m_position <  rhs.m_position; }
    bool operator<= (const This& rhs) const { return m_position <= rhs.m_position; }
    bool operator>  (const This& rhs) const { return m_position >  rhs.m_position; }
    bool operator>= (const This& rhs) const { return m_position >= rhs.m_position; }

    This& operator++() { ++m_position; return *this; }
    This  operator++(int) { This n = *this; advance(1); return n; }
    This& operator+=(size_t i) { advance(i); return *this; }

private:
    void advance(size_t n)
    {
        for (size_t i = 0; i < n; i++)
            m_functor();

        ++m_position;
    }

private:
    Func m_functor;
    size_t m_position {};
};

template<typename Func> functor_iterator(Func) -> functor_iterator<Func>;
