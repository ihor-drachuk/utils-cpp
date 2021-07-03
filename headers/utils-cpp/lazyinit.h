#pragma once
#include <functional>

struct LazyInitBase
{
    enum no_init { no_init_tag };
};

template<typename T>
class LazyInit : public LazyInitBase
{
public:
    template<typename Factory>
    LazyInit(const Factory& factory): m_factory(factory) { }
    LazyInit(no_init) { }
    LazyInit(const LazyInit<T>&) = delete;
    ~LazyInit() { delete m_content; }

    LazyInit<T>& operator=(const LazyInit<T>&) = delete;

    template<typename Factory>
    void setFactory(const Factory& factory) { m_factory = factory; }

    operator bool() const { return m_content != nullptr; }

    T* operator->() { create(); return m_content; }
    const T* operator->() const { create(); return m_content; }
    T& operator*() { create(); return *m_content; }
    const T& operator*() const { create(); return *m_content; }

private:
    void create() const {
        if (!m_content)
            m_content = m_factory();
    }

private:
    std::function<T*()> m_factory;
    mutable T* m_content { nullptr };
};
