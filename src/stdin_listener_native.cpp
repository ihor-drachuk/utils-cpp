/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/utils-qt
 * Contact:  ihor-drachuk-libs@pm.me  */

#ifdef UTILS_CPP_OS_WINDOWS
#ifdef UTILS_CPP_COMPILER_MINGW
#define _WIN32_WINNT 0x0601 // NOLINT
#endif // UTILS_CPP_COMPILER_MINGW
#endif // UTILS_CPP_OS_WINDOWS

#include <utils-cpp/stdin_listener_native.h>

#include <atomic>
#include <cassert>
#include <string>
#include <thread>
#include <vector>

#ifdef UTILS_CPP_OS_WINDOWS
#include <Windows.h>
#else
#include <poll.h>
#include <unistd.h>
#endif // UTILS_CPP_OS_WINDOWS

namespace utils_cpp {

struct StdinListenerNative::impl_t
{
    NewLineHandler lineCallback;

    std::thread thread;
    std::atomic<bool> running{true};

    std::string lineBuffer;
};

StdinListenerNative::StdinListenerNative(const NewLineHandler& lineCallback)
{
    createImpl();
    assert(lineCallback);

    impl().lineCallback = lineCallback;
    impl().thread = std::thread([this] { readLoop(); });
}

StdinListenerNative::~StdinListenerNative()
{
    impl().running = false;

    if (impl().thread.joinable())
        impl().thread.join();
}

void StdinListenerNative::readLoop()
{
    while (impl().running) {
        if (!waitForData(100))
            continue;

        char buf[1024];
        const auto bytesRead = readAvailable(buf, sizeof(buf));

        if (bytesRead <= 0)
            break;

        // Accumulate into line buffer, dispatch complete lines
        for (size_t i = 0; i < static_cast<size_t>(bytesRead); ++i) {
            const char ch = buf[i];

            if (ch == '\n') {
                // Strip trailing \r for CRLF
                if (!impl().lineBuffer.empty() && impl().lineBuffer.back() == '\r')
                    impl().lineBuffer.pop_back();

                impl().lineCallback(impl().lineBuffer);
                impl().lineBuffer.clear();
            } else {
                impl().lineBuffer += ch;
            }
        }
    }
}

bool StdinListenerNative::waitForData(int timeoutMs)
{
#ifdef UTILS_CPP_OS_WINDOWS
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    return WaitForSingleObject(hStdin, static_cast<DWORD>(timeoutMs)) == WAIT_OBJECT_0;
#else
    struct pollfd pfd {};
    pfd.fd = STDIN_FILENO;
    pfd.events = POLLIN;
    return poll(&pfd, 1, timeoutMs) > 0;
#endif // UTILS_CPP_OS_WINDOWS
}

int StdinListenerNative::readAvailable(char* buffer, size_t sz)
{
#ifdef UTILS_CPP_OS_WINDOWS
    DWORD bytesRead {};
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    if (!ReadFile(hStdin, buffer, static_cast<DWORD>(sz), &bytesRead, nullptr))
        return -1;
    return static_cast<int>(bytesRead);
#else
    return static_cast<int>(read(STDIN_FILENO, buffer, sz));
#endif // UTILS_CPP_OS_WINDOWS
}

} // namespace utils_cpp
