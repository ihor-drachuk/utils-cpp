/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/utils-qt
 * Contact:  ihor-drachuk-libs@pm.me  */

#include <utils-cpp/stdin_listener_native.h>

#include <atomic>
#include <cassert>
#include <chrono>
#include <iostream>
#include <thread>

#ifdef UTILS_CPP_OS_WINDOWS
#include <Windows.h>
#else
#include <unistd.h>
#endif // UTILS_CPP_OS_WINDOWS

namespace utils_cpp {

struct StdinListenerNative::impl_t
{
    NewLineHandler lineCallback;

    std::thread thread;
    std::atomic<bool> running{true};
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

#ifdef UTILS_CPP_OS_WINDOWS
    // On Windows we can cancel pending I/O operations
    HANDLE stdinHandle = GetStdHandle(STD_INPUT_HANDLE);
    if (stdinHandle != INVALID_HANDLE_VALUE) {
        CancelIoEx(stdinHandle, nullptr);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        CancelIoEx(stdinHandle, nullptr);
    }

#else
    // On POSIX systems, close() on stdin's fd will interrupt blocking read
    int fd = STDIN_FILENO;  // or fileno(stdin)
    close(dup(fd));  // Duplicate and close to avoid closing the original fd
#endif // UTILS_CPP_OS_WINDOWS

    if (impl().thread.joinable())
        impl().thread.join();
}

void StdinListenerNative::readLoop()
{
    std::string line;

    while (impl().running) {
        bool gotLine = static_cast<bool>(std::getline(std::cin, line));

        if (!gotLine)
            break;

        impl().lineCallback(line);
    }
}

} // namespace utils_cpp
