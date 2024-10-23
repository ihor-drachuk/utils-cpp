/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/utils-qt
 * Contact:  ihor-drachuk-libs@pm.me  */

#pragma once
#include <functional>
#include <string>
#include <utils-cpp/pimpl.h>

namespace utils_cpp {

class StdinListener
{
public:
    using NewDataHandler = std::function<void(const char* buffer, size_t size)>;
    using NewLineHandler = std::function<void(const std::string& str)>;
    enum class EchoMode { Auto, Off, On };
    enum class NewLineMode { Any, CRLF, CR, LF };

    StdinListener(EchoMode mode, NewLineMode newlineMode, const NewDataHandler& dataCallback, const NewLineHandler& lineCallback);
    ~StdinListener();

private:
    EchoMode resolveEchoMode(EchoMode mode) const;
    void echo(const char* buffer, size_t size);
    void searchLines(size_t offset);
    void listenForInput();
    void setNonBlockingMode();
    void resetBlockingMode();
    size_t readNonBlocking(char* buffer, size_t sz);

private:
    DECLARE_PIMPL
};

} // namespace utils_cpp
