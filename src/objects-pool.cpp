#include "utils-cpp/objects-pool.h"

#include <vector>
#include <mutex>
#include <condition_variable>
#include <algorithm>
#include <atomic>

using LockPtr = std::shared_ptr<std::unique_lock<std::mutex>>;


struct ObjectContext
{
    std::shared_ptr<void> obj;
    std::unique_ptr<std::atomic_bool> isUsed = std::make_unique<std::atomic_bool>(false);
};


struct ObjectsPoolBase::impl_t
{
    std::vector<ObjectContext> objects;
    std::atomic_int available { 0 };
    std::mutex mutex;
    std::condition_variable cv;

    LockPtr lock() { return std::make_shared<std::unique_lock<std::mutex>>(mutex); }
};


ObjectsPoolBase::ObjectsPoolBase()
{
    createImpl();
}

ObjectsPoolBase::~ObjectsPoolBase()
{
}

void ObjectsPoolBase::baseAppend(const std::shared_ptr<void>& obj)
{
    auto lck = impl().lock();
    impl().objects.emplace_back(ObjectContext{obj});
    impl().available++;
    impl().cv.notify_one();
}

const std::shared_ptr<void> ObjectsPoolBase::baseTake()
{
    auto lck = impl().lock();

    if (!impl().available)
        impl().cv.wait(*lck, [this]() -> bool { return (impl().available > 0); });

    auto it = std::find_if(impl().objects.begin(), impl().objects.end(), [](const ObjectContext& ctx) -> bool { return !*ctx.isUsed; });
    assert(it != impl().objects.end());

    *it->isUsed = true;
    impl().available--;

    return it->obj;
}

void ObjectsPoolBase::returnObject(void* ptr)
{
    auto lck = impl().lock();
    auto it = std::find_if(impl().objects.begin(), impl().objects.end(), [ptr](const ObjectContext& ctx) -> bool { return (ctx.obj.get() == ptr); });
    assert(it != impl().objects.end());

    *it->isUsed = false;
    impl().available++;
    impl().cv.notify_one();
}

ObjectAccessorBase::ObjectAccessorBase(const std::shared_ptr<void>& obj, const std::shared_ptr<ObjectsPoolBase>& master)
    : object(obj), master(master)
{
}

ObjectAccessorBase::~ObjectAccessorBase()
{
    master->returnObject(object.get());
}

void* ObjectAccessorBase::get_internal()
{
    return object.get();
}

const void* ObjectAccessorBase::get_internal() const
{
    return object.get();
}
