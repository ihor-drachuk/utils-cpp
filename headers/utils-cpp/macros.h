/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/utils-cpp
 * Contact:  ihor-drachuk-libs@pm.me  */

#define STRINGIFY(x) #x
#define TO_STRING(x) STRINGIFY(x)

#define COND_ACT_RETURN(Condition, Action, ReturnValue) \
    do { \
        if (Condition) { \
            Action; \
            return ReturnValue; \
        } \
    } while (0)
