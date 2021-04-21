#include "utils-cpp/threadid.h"
#include <atomic>

uintmax_t currentThreadId()
{
    static std::atomic_uintmax_t counter;
    thread_local uintmax_t threadId = counter.fetch_add((uintmax_t)1);
    return threadId;
}
