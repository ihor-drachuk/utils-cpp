#pragma once
#include <utility>
#include <memory>
#include <cassert>

// Notice!
// It requires in .cpp file:
//   1. Explicit destructor
//   2. "struct T::impl_t"

#define DECLARE_PIMPL  \
    struct impl_t; \
    std::unique_ptr<impl_t> _impl; \
    impl_t& impl() { assert(_impl); return *_impl; } \
    const impl_t& impl() const { assert(_impl); return *_impl; } \
    template<typename... Args> \
    void createImpl(Args&&... args) { assert(!_impl); _impl = std::make_unique<impl_t>(std::forward<Args>(args)...); }
