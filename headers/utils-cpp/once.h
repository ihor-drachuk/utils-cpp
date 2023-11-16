/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/utils-cpp
 * Contact:  ihor-drachuk-libs@pm.me  */

#pragma once

//! Provides ability to execute code only once.
//! Be careful, it uses static flag that can be common for all instances of class.

#define ONCE_NAMED(flag, code) \
    static bool _flag##flag = true; \
    if (_flag##flag) { \
        _flag##flag = false; \
        code; \
    }

#define ONCE(code) ONCE_NAMED(flag, code)

#define ONCE_GLOBAL_NAMED(flag, code) \
    namespace { \
    struct _OnceGlobal##flag { \
        _OnceGlobal##flag() { \
            code; \
        } \
    }; \
    _OnceGlobal##flag _onceGlobal##flag; \
    } /* namespace */

#define ONCE_GLOBAL(code) ONCE_GLOBAL_NAMED(flag, code)
