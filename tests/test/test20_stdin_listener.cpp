/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/utils-cpp
 * Contact:  ihor-drachuk-libs@pm.me  */

#include <gtest/gtest.h>
#include <utils-cpp/stdin_listener.h>
#include <utils-cpp/stdin_listener_native.h>
#include <chrono>
#include <thread>

// These tests verify that constructing and destroying StdinListener /
// StdinListenerNative does not hang when stdin has no data (which is the
// normal case when running under CTest or an IDE).  If the destructor
// blocks, the test will time out and be killed by CTest.

TEST(utils_cpp, StdinListener_destructor_does_not_hang)
{
    {
        utils_cpp::StdinListener listener(
            utils_cpp::StdinListener::EchoMode::Off,
            utils_cpp::StdinListener::NewLineMode::Any,
            nullptr,
            [](const std::string&) {}
        );
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    // If we reach here, the destructor didn't hang
    SUCCEED();
}

TEST(utils_cpp, StdinListenerNative_destructor_does_not_hang)
{
    {
        utils_cpp::StdinListenerNative listener(
            [](const std::string&) {}
        );
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    // If we reach here, the destructor didn't hang
    SUCCEED();
}
