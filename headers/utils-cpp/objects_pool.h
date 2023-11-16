/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/utils-cpp
 * Contact:  ihor-drachuk-libs@pm.me  */

#pragma once
#include <type_traits>
#include <memory>
#include <cassert>
#include <utils-cpp/pimpl.h>

class ObjectsPoolBase;
template<typename T> class ObjectsPool;
template<typename T> using ObjectsPoolPtr = std::shared_ptr<ObjectsPool<T>>;


class ObjectAccessorBase
{
public:
    virtual ~ObjectAccessorBase();

protected:
    ObjectAccessorBase(const std::shared_ptr<void>& obj,
                       const std::shared_ptr<ObjectsPoolBase>& master);

    void* get_internal();
    const void* get_internal() const;

private:
    std::shared_ptr<void> object;
    std::shared_ptr<ObjectsPoolBase> master;
};


template<typename T>
class ObjectAccessor : public ObjectAccessorBase
{
    template<typename> friend class ObjectsPool;
    using ObjectAccessorBase::ObjectAccessorBase;
public:
    T& ref() { return *reinterpret_cast<T*>(get_internal()); }
    const T& ref() const { return *reinterpret_cast<const T*>(get_internal()); }

    T& operator*() { return ref(); }
    const T& operator*() const { return ref(); }
    T* operator->() { return &ref(); }
    const T* operator->() const { return &ref(); }
    T* get() { return &ref(); }
    const T* get() const { return &ref(); }
};


class ObjectsPoolBase : public std::enable_shared_from_this<ObjectsPoolBase>
{
    friend class ObjectAccessorBase;
public:
    ObjectsPoolBase();
    virtual ~ObjectsPoolBase();

protected:
    void baseAppend(const std::shared_ptr<void>& obj);
    const std::shared_ptr<void> baseTake();

private:
    void returnObject(void* ptr);

private:
    DECLARE_PIMPL
};


template<typename T>
class ObjectsPool : public ObjectsPoolBase
{
public:
    template<typename... Args>
    static ObjectsPoolPtr<T> create(int count = 1, Args&&... args) {
        assert(count >= 0);
        auto pool = std::shared_ptr<ObjectsPool>(new ObjectsPool, [](ObjectsPool* p){ delete p; });

        if (count > 0)
            pool->append(count, std::forward<Args>(args)...);

        return pool;
    }

    template<typename Deleter = std::nullptr_t>
    void append(T* object, Deleter deleter = nullptr) {
        if constexpr (std::is_same<Deleter, std::nullptr_t>::value) {
            append(std::shared_ptr<T>(object));
        } else {
            append(std::shared_ptr<T>(object, deleter));
        }
    }

    template<typename... Args>
    void append(int count, Args&&... args) {
        assert (count > 0);

        for (int i = 0; i < count; i++)
            append(std::make_shared<T>(std::forward<Args>(args)...));
    }

    template<typename... Args, typename Deleter>
    void appendWithDeleter(int count, Args&&... args, Deleter deleter) {
        assert (count > 0);

        for (int i = 0; i < count; i++)
            append(std::shared_ptr<T>(new T(std::forward<Args>(args)...), deleter));
    }

    ObjectAccessor<T> take() {
        const auto obj = baseTake();
        ObjectAccessor<T> accessor(obj, shared_from_this());
        return accessor;
    }

private:
    ObjectsPool() {}
    ~ObjectsPool() {};

    void append(const std::shared_ptr<T>& obj) {
        std::shared_ptr<void> ptr = std::static_pointer_cast<void>(obj);
        baseAppend(ptr);
    }
};

