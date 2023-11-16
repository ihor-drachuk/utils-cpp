/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/utils-cpp
 * Contact:  ihor-drachuk-libs@pm.me  */

#pragma once
#include <memory>

template<typename T>
std::shared_ptr<T> make_shared_from(const T& rhs) { return std::make_shared<T>(rhs); }

template<typename T>
std::unique_ptr<T> make_unique_from(const T& rhs) { return std::make_unique<T>(rhs); }
