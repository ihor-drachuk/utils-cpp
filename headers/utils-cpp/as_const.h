/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/utils-cpp
 * Contact:  ihor-drachuk-libs@pm.me  */

#pragma once

namespace utils_cpp {

template<typename T>
const auto& as_const(T& value) { return value; }

} // namespace utils_cpp
