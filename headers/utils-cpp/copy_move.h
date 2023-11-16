/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/utils-cpp
 * Contact:  ihor-drachuk-libs@pm.me  */

#pragma once

// These are helper-macros for the rule of 3/5
// https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rc-five

#define NO_COPY(classname)                \
   classname(const classname &) = delete; \
   classname &operator=(const classname &) = delete

#define NO_MOVE(classname)           \
   classname(classname &&) = delete; \
   classname &operator=(classname &&) = delete

#define NO_COPY_MOVE(classname) \
   NO_COPY(classname);          \
   NO_MOVE(classname)

#define DEFAULT_COPY(classname)            \
   classname(const classname &) = default; \
   classname &operator=(const classname &) = default

#define DEFAULT_MOVE(classname)       \
   classname(classname &&) noexcept = default; \
   classname &operator=(classname &&) noexcept = default

#define DEFAULT_COPY_MOVE(classname) \
   DEFAULT_COPY(classname);          \
   DEFAULT_MOVE(classname)
