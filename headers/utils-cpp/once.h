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
