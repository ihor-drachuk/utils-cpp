/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/utils-cpp
 * Contact:  ihor-drachuk-libs@pm.me  */

#pragma once
#include <memory>

template<typename T>
std::shared_ptr<T> make_shared_from(const T& value) { return std::make_shared<T>(value); }

template<typename T>
std::unique_ptr<T> make_unique_from(const T& value) { return std::make_unique<T>(value); }

template<typename T, typename... Args>
std::shared_ptr<T> wrap_by_shared_ptr(T* ptr, Args&&... args) { return std::shared_ptr<T>(ptr, std::forward<Args>(args)...); }

template<typename T, typename... Args>
std::unique_ptr<T, Args...> wrap_by_unique_ptr(T* ptr, Args&&... args) { return std::unique_ptr<T, Args...>(ptr, std::forward<Args>(args)...); }
