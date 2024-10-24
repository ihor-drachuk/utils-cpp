/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/utils-qt
 * Contact:  ihor-drachuk-libs@pm.me  */
#include <utils-cpp/stdin_listener.h>

#include <algorithm>
#include <array>
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

const auto stopChecker = [](char x){
    static constexpr auto lookup = [](){
        std::array<unsigned char, 256> arr{};
        arr[0x08] = 1;
        arr[0x7F] = 1;
        arr['\r'] = 1;
        return arr;
    }();

    return lookup[static_cast<unsigned char>(x)];
};

const auto bkspChecker = [](char x){
    static constexpr auto lookup = [](){
        std::array<unsigned char, 256> arr{};
        arr[0x08] = 1;
        arr[0x7F] = 1;
        return arr;
    }();

    return lookup[static_cast<unsigned char>(x)];
};

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
        auto begin = buffer;
        const auto end = buffer + size;

        auto it = std::find_if(begin, end, stopChecker);
        while (it != end) {
            size_t length = static_cast<size_t>(it - begin);

            if (*it == '\r') {
                fwrite(begin, 1, length + 1, stdout);
                fputc('\n', stdout);

            } else {
                assert(*it == 0x08 || *it == 0x7F);
                fwrite(begin, 1, length, stdout);
                fwrite("\b \b", 1, 3, stdout);
            }

            begin = it + 1;
            it = std::find_if(begin, end, stopChecker);
        }

        size_t length = static_cast<size_t>(end - begin);
        fwrite(begin, 1, length, stdout);
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
        if (const auto size = readNonBlocking(buffer, sizeof(buffer))) {
            echo(buffer, size);

            if (impl().newDataCallback)
                impl().newDataCallback(buffer, size);

            if (impl().newLineCallback) {
                auto begin = buffer;
                const auto end = buffer + size;
                size_t nlSearchOffset = impl().buffer.size();

                impl().buffer.reserve(impl().buffer.size() + size);

                auto it = std::find_if(begin, end, bkspChecker);
                while (it != end) {
                    assert(*it == 0x08 || *it == 0x7F);
                    auto length = std::distance(begin, it);

                    // it - points to started backspaces sequence
                    // continueIt - points to the next character after backspaces sequence

                    auto continueIt = std::find_if_not(it + 1, end, bkspChecker);
                    const auto backspacesCount = std::distance(it, continueIt);
                    length -= backspacesCount; // don't copy symbols we're going to delete

                    if (length < 0) {
                        // If there are more backspaces than characters to copy,
                        // then remove rest from buffer, if there are enough data collected
                        const auto rest = (std::min)(static_cast<size_t>(-length), impl().buffer.size());
                        const auto newSize = impl().buffer.size() - rest;
                        impl().buffer.resize(newSize);
                        if (newSize < nlSearchOffset) // We can't search from previous buffer end, as we shortened it
                            nlSearchOffset = newSize;

                    } else {
                        // length >= 0, so there is some data left after applied backspaces
                        const auto lengthSzt = static_cast<size_t>(length);
                        const auto offset = impl().buffer.size();
                        impl().buffer.resize(offset + lengthSzt);
                        memcpy(impl().buffer.data() + offset, begin, lengthSzt);
                    }

                    begin = continueIt;
                    it = std::find_if(begin, end, bkspChecker);
                }

                const auto restSize = static_cast<size_t>(std::distance(begin, end));
                const auto offset = impl().buffer.size();
                impl().buffer.resize(offset + restSize);
                memcpy(impl().buffer.data() + offset, begin, restSize);

                searchLines(nlSearchOffset);
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
