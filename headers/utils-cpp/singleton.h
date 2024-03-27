/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/utils-cpp
 * Contact:  ihor-drachuk-libs@pm.me  */

#pragma once
#include <cassert>

#include "attributes.h"

template<class T, bool IsGlobal>
class SingletonImpl
{
public:
    // The UB-San complains because at the time of this call T's ctor has not yet been called.
    SingletonImpl() UTIL_CPP_ATTR_UB
    {
        assert(!m_instance);
        m_instance = static_cast<T*>(this);
    }

    ~SingletonImpl()
    {
        assert(m_instance);
        m_instance = nullptr;
    }

    static T* instance()
    {
        static_assert(IsGlobal, "'instance' method is not available for non-global singletons!");
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

template<class T, bool IsGlobal>
T* SingletonImpl<T, IsGlobal>::m_instance = nullptr;

template<class T>
class Singleton : public SingletonImpl<T, false>
{};

template<class T>
class SingletonGlobal : public SingletonImpl<T, true>
{};
