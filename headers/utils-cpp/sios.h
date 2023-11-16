/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/utils-cpp
 * Contact:  ihor-drachuk-libs@pm.me  */

#pragma once
#include <memory>
#include <atomic>
#include <utils-cpp/singleton.h>

// Static initialization order solution
// Solution for "static initialization order fiasco" problem

template<typename T>
class SIOS : public Singleton<SIOS<T>>
{
public:
    template<typename... Args>
    SIOS(Args&&... args) {
        m_object = std::make_shared<T>(std::forward<Args>(args)...);
    }

    std::shared_ptr<T> get() {
        if (!m_ready) return {};
        return m_object;
    }

    T* operator->() { return m_object.get(); }
    const T* operator->() const { return m_object.get(); }
    T& operator*() { return *m_object.get(); }
    const T& operator*() const { return *m_object.get(); }

    bool isReady() const { return m_ready; }
    void markReady() { m_ready = true; }

private:
    std::atomic_bool m_ready { false };
    std::shared_ptr<T> m_object;
};


template<typename T>
class SIOS_Entry
{
public:
    SIOS_Entry() {
        m_created = true;
        renew();
    }

    SIOS_Entry(const SIOS_Entry<T>& rhs) {
        m_object = rhs.m_object;
        m_created = true;
    }

    const std::shared_ptr<T>& get() {
        assert(m_created);
        renew();
        return m_object;
    }

    const std::shared_ptr<T>& fastGet() {
        assert(m_created);
        return m_object;
    }

    bool available() {
        assert(m_created);
        renew();
        return !!m_object;
    }

private:
    void renew() {
        assert(m_created);
        if (m_object) return;
        if (!SIOS<T>::exists()) return;
        if (!SIOS<T>::instance()->isReady()) return;
        m_object = SIOS<T>::instance()->get();
    }

private:
    bool m_created { false };
    std::shared_ptr<T> m_object;
};
