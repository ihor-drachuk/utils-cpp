/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/utils-cpp
 * Contact:  ihor-drachuk-libs@pm.me  */

#pragma once
#include <utility>

template<typename Functor>
class scoped_guard {
public:
    scoped_guard(Functor f): m_f(f) {}
    scoped_guard(const scoped_guard&) = delete;
    scoped_guard(scoped_guard&& rhs) noexcept: m_f(std::move(rhs.m_f)) { rhs.m_moved = true; }
    ~scoped_guard() { if (!m_moved && !m_reset) m_f(); }

    scoped_guard<Functor>& operator=(const scoped_guard<Functor>&) = delete;
    scoped_guard<Functor>& operator=(scoped_guard<Functor>&& rhs) noexcept {
        if (this == &rhs) return *this;

        m_f = rhs.m_f;
        m_reset = rhs.m_reset;
        rhs.m_moved = true;

        return *this;
    }

    void reset() { m_reset = true; }

private:
    Functor m_f;
    bool m_moved { false };
    bool m_reset { false };
};

template<typename Functor>
scoped_guard<Functor> CreateScopedGuard(Functor f) { return scoped_guard<Functor>(f); }
