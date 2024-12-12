/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/utils-cpp
 * Contact:  ihor-drachuk-libs@pm.me  */

#define STRINGIFY(x) #x
#define TO_STRING(x) STRINGIFY(x)

#define COND_ACT_RETURN_VAL(Condition, Action, ReturnValue) \
    do { \
        if (Condition) { \
            Action; \
            return ReturnValue; \
        } \
    } while (0)

#define COND_ACT_RETURN(Condition, Action) \
do { \
        if (Condition) { \
            Action; \
            return; \
    } \
} while (0)
