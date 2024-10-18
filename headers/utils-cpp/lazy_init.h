/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/utils-cpp
 * Contact:  ihor-drachuk-libs@pm.me  */

#pragma once
#include <functional>
#include <cassert>

namespace utils_cpp {

class lazy_init_types
{
public:
    enum no_init { no_init_tag };
};

template<typename T>
class lazy_init_base : public lazy_init_types
{
public:
    lazy_init_base() = default;

    template<typename Factory>
    lazy_init_base(const lazy_init_base<T>&) = delete;
    ~lazy_init_base() { delete m_content; }

    lazy_init_base<T>& operator=(const lazy_init_base<T>&) = delete;

    explicit operator bool() const { return m_content != nullptr; }

    T* operator->() { ensure_init(); return m_content; }
    const T* operator->() const { ensure_init(); return m_content; }
    T& operator*() { ensure_init(); return *m_content; }
    const T& operator*() const { ensure_init(); return *m_content; }

protected:
    virtual T* create() const = 0;

private:
    void ensure_init() const {
        if (!m_content) {
            m_content = create();
            assert(m_content && "'create()' returned nullptr!");
        }
    }

private:
    mutable T* m_content {};
};


template<typename T>
class lazy_init : public lazy_init_base<T>
{
public:
    using lazy_init_base<T>::lazy_init_base;

protected:
    T* create() const override { return new T(); }
};


template<typename T>
class lazy_init_custom : public lazy_init_base<T>
{
public:
    template<typename Factory>
    lazy_init_custom(const Factory& factory): m_factory(factory) {}

    lazy_init_custom(lazy_init_types::no_init) {}

    void setFactory(const std::function<T*()>& factory) {
        assert(!m_factory && factory);
        m_factory = factory;
    }

protected:
    T* create() const override {
        assert(m_factory);
        return m_factory();
    }

private:
    std::function<T*()> m_factory;
};

} // namespace utils_cpp
