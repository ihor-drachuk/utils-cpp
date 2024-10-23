/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/utils-qt
 * Contact:  ihor-drachuk-libs@pm.me  */

#include <utils-cpp/stdin_listener.h>

#include <algorithm>
#include <atomic>
#include <cassert>
#include <cstring>
#include <thread>
#include <vector>

#ifdef _WIN32
#include <Windows.h>
#include <io.h>
#define isatty _isatty
#define fileno _fileno
using ConsoleMode = DWORD;
#else
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
using ConsoleMode = int;
#endif // _WIN32

namespace utils_cpp {

namespace {

struct NewlineVariant
{
    StdinListener::NewLineMode mode {};
    std::vector<char> ending;
};

} // namespace

struct StdinListener::impl_t
{
    EchoMode echoMode;
    NewLineMode newlineMode;
    NewDataHandler newDataCallback;
    NewLineHandler newLineCallback;

    std::thread listenerThread;
    std::atomic<bool> stopFlag {};

    const std::vector<NewlineVariant> newlineVariants {
        {NewLineMode::CRLF, {'\r', '\n'}},
        {NewLineMode::CR,   {'\r'}},
        {NewLineMode::LF,   {'\n'}}
    };

    ConsoleMode consoleMode {};
    std::vector<char> buffer;
};

StdinListener::StdinListener(EchoMode mode, NewLineMode newlineMode, const NewDataHandler& dataCallback, const NewLineHandler& lineCallback)
{
    createImpl();
    impl().echoMode = resolveEchoMode(mode);
    impl().newlineMode = newlineMode;
    impl().newDataCallback = dataCallback;
    impl().newLineCallback = lineCallback;
    impl().listenerThread = std::thread(&StdinListener::listenForInput, this);
}

StdinListener::~StdinListener()
{
    impl().stopFlag = true;

    if (impl().listenerThread.joinable())
        impl().listenerThread.join();
}

StdinListener::EchoMode StdinListener::resolveEchoMode(EchoMode mode) const
{
    return mode == EchoMode::Auto ? (isatty(fileno(stdin)) ? EchoMode::On : EchoMode::Off) :
                                    mode;
}

void StdinListener::echo(const char* buffer, size_t size)
{
    assert(impl().echoMode != EchoMode::Auto);
    assert(size > 0);

    if (impl().echoMode == EchoMode::On) {
        fwrite(buffer, 1, size, stdout);
        if (buffer[size-1] == '\r')
            putchar('\n');
        fflush(stdout);
    }
}

void StdinListener::searchLines(size_t offset)
{
    auto begin = impl().buffer.begin();
    const auto end = impl().buffer.end();

    while (begin != end) {
        auto minIt = end;
        size_t delta = 0;

        for (const auto& newlineVariant : impl().newlineVariants) {
            if (impl().newlineMode != StdinListener::NewLineMode::Any && impl().newlineMode != newlineVariant.mode)
                continue;

            auto it = std::search(begin + static_cast<long long>(offset), end, newlineVariant.ending.begin(), newlineVariant.ending.end());

            if (it != end && it < minIt) {
                minIt = it;
                delta = newlineVariant.ending.size();
            }
        }

        if (minIt != end) {
            impl().newLineCallback(std::string(begin, minIt));
            begin = minIt + static_cast<long long>(delta);
            offset = 0;
        } else {
            break;
        }
    }

    if (begin != end) {
        assert(begin < end);
        const auto remaining = static_cast<size_t>(end - begin);
        std::copy(begin, end, impl().buffer.begin());
        impl().buffer.resize(remaining);
    } else {
        impl().buffer.clear();
    }
}

void StdinListener::listenForInput()
{
    setNonBlockingMode();

    char buffer[1024];

    while (!impl().stopFlag) {
        if (auto size = readNonBlocking(buffer, sizeof(buffer))) {
            echo(buffer, size);

            if (impl().newDataCallback)
                impl().newDataCallback(buffer, size);

            if (impl().newLineCallback) {
                const auto offset = impl().buffer.size();
                impl().buffer.resize(offset + size);
                memcpy(impl().buffer.data() + offset, buffer, size);
                searchLines(offset);
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(30));
    }

    resetBlockingMode();
}

void StdinListener::setNonBlockingMode()
{
#ifdef _WIN32
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    GetConsoleMode(hStdin, &impl().consoleMode);
    SetConsoleMode(hStdin, impl().consoleMode & ~ENABLE_LINE_INPUT);
#else
    impl().consoleMode = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, impl().consoleMode | O_NONBLOCK);
#endif // _WIN32
}

void StdinListener::resetBlockingMode()
{
#ifdef _WIN32
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    SetConsoleMode(hStdin, impl().consoleMode);
#else
    fcntl(STDIN_FILENO, F_SETFL, impl().consoleMode);
#endif // _WIN32
}

size_t StdinListener::readNonBlocking(char* buffer, size_t sz)
{
#ifdef _WIN32
    DWORD bytesRead {};
    const auto status = ReadFile(GetStdHandle(STD_INPUT_HANDLE), buffer, sz, &bytesRead, nullptr);
    return status ? bytesRead : 0;
#else
    return read(STDIN_FILENO, buffer, sz);
#endif // _WIN32
}

} // namespace utils_cpp
