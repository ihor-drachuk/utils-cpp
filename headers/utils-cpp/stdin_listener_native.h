/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/utils-qt
 * Contact:  ihor-drachuk-libs@pm.me  */

#pragma once
#include <functional>
#include <string>
#include <utils-cpp/pimpl.h>

namespace utils_cpp {

class StdinListenerNative
{
public:
    using NewLineHandler = std::function<void(const std::string& str)>;

    StdinListenerNative(const NewLineHandler& lineCallback);
    ~StdinListenerNative();

private:
    void readLoop();
    static bool waitForData(int timeoutMs);
    static int readAvailable(char* buffer, size_t sz);

private:
    DECLARE_PIMPL
};

} // namespace utils_cpp
