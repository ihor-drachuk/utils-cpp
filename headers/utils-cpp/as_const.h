#pragma once
#include <type_traits>

namespace utils_cpp {

template<typename T>
const auto& as_const(T& value) { return const_cast<std::add_const_t<T>&>(value); }

} // namespace utils_cpp
