/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/utils-cpp
 * Contact:  ihor-drachuk-libs@pm.me  */

#pragma once
#include <cassert>
#include "attributes.h"



template<class T>
class Singleton
{
public:
    // The UB-San complains because at the time of this call T's ctor has not yet been called.
    Singleton() UTIL_CPP_ATTR_UB
    {
        assert(!m_instance);
        m_instance = static_cast<T*>(this);
    }

    ~Singleton()
    {
        assert(m_instance);
        m_instance = nullptr;
    }

    static T* instance()
    {
        assert(m_instance);
        return m_instance;
    }

    static bool exists()
    {
        return m_instance;
    }

private:
    static T* m_instance;
};

template<class T>
T* Singleton<T>::m_instance = nullptr;
