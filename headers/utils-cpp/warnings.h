/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/utils-cpp
 * Contact:  ihor-drachuk-libs@pm.me  */

#include <utils-cpp/macros.h>

#if defined(_MSC_VER)
    #define WARNINGS_PUSH  _Pragma("warning(push)")
#elif defined(__clang__)
    #define WARNINGS_PUSH  _Pragma("clang diagnostic push")
#elif defined(__GNUC__)
    #define WARNINGS_PUSH  _Pragma("GCC diagnostic push")
#endif

#if defined(_MSC_VER)
    #define SUPPRESS_WARNING(GccVariant, ClangVariant, MsvcVariant)  __pragma(warning(disable : MsvcVariant))
#elif defined(__clang__)
    #define SUPPRESS_WARNING(GccVariant, ClangVariant, MsvcVariant)  _Pragma(TO_STRING(clang diagnostic ignored ClangVariant))
#elif defined(__GNUC__)
    #define SUPPRESS_WARNING(GccVariant, ClangVariant, MsvcVariant)  _Pragma(TO_STRING(GCC diagnostic ignored GccVariant))
#endif

#if defined(_MSC_VER)
    #define WARNINGS_POP  _Pragma("warning(pop)")
#elif defined(__clang__)
    #define WARNINGS_POP  _Pragma("clang diagnostic pop")
#elif defined(__GNUC__)
    #define WARNINGS_POP  _Pragma("GCC diagnostic pop")
#endif
