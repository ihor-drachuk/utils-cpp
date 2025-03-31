/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/utils-cpp
 * Contact:  ihor-drachuk-libs@pm.me  */

#include <cctype>
#include <benchmark/benchmark.h>
#include <utils-cpp/lazy_init.h>
#include <utils-cpp/data_to_string.h>
#include <utils-cpp/xor.h>
#include <utils-cpp/functor_iterator.h>
#include <utils-cpp/container_utils.h>
#include "internal/data_10kb.h"

static void benchmark_stub(benchmark::State& state)
{
    int i = 0;

    while (state.KeepRunning())
        i++;

    (void)i;
}

BENCHMARK(benchmark_stub);


static void benchmark_lazy_init(benchmark::State& state)
{
    while (state.KeepRunning()) {
        utils_cpp::lazy_init<std::string> li;
        (void)li;
    }
}

BENCHMARK(benchmark_lazy_init);


static void benchmark_lazy_init_off(benchmark::State& state)
{
    while (state.KeepRunning()) {
        std::string str;
        (void)str;
    }
}

BENCHMARK(benchmark_lazy_init_off);


static void benchmark_data_to_string(benchmark::State& state)
{
    std::string someData = "1234My567Data12";

    for (size_t i = 0; i < someData.size(); i++)
        if (std::isdigit(someData[i]))
            someData[i] = i % 10;

    while (state.KeepRunning())
        (void)utils_cpp::data_to_string(someData.data(), someData.size());
}

BENCHMARK(benchmark_data_to_string);


static void benchmark_xor_bytes(benchmark::State& state)
{
    auto buffer = data_10kb_1();
    const auto mask = data_10kb_2();

    while (state.KeepRunning()) {
        auto itDst = buffer.begin();
        auto itDstEnd = buffer.end();
        auto itSrc = mask.begin();

        while (itDst != itDstEnd)
            *itDst++ ^= *itSrc++;
    }

    (void)buffer;
}

BENCHMARK(benchmark_xor_bytes);


static void benchmark_xor_buffer(benchmark::State& state)
{
    auto buffer = data_10kb_1();
    const auto mask = data_10kb_2();

    while (state.KeepRunning())
        utils_cpp::xorBuffer(buffer.data(), mask.data(), buffer.size());

    (void)buffer;
}

BENCHMARK(benchmark_xor_buffer);


static void benchmark_xor_bufferIt(benchmark::State& state)
{
    auto buffer = data_10kb_1();
    const auto mask = data_10kb_2();

    auto functor = [it = mask.begin()]() mutable { return *it++; };

    while (state.KeepRunning())
        utils_cpp::xorBufferIt(buffer.data(), buffer.data() + buffer.size(), functor_iterator(functor));

    (void)buffer;
}

BENCHMARK(benchmark_xor_bufferIt);


static void benchmark_random_weighted_items_10(benchmark::State& state)
{
    constexpr size_t Size = 10;
    constexpr size_t Count = 1000;
    const std::vector<int> items = utils_cpp::generate<std::vector>(Size, [cnt = 0]() mutable { return ++cnt; });

    while (state.KeepRunning()) {
        (void)utils_cpp::random_weighted_items(items, Count);
    }
}

BENCHMARK(benchmark_random_weighted_items_10);


static void benchmark_random_weighted_items_100(benchmark::State& state)
{
    constexpr size_t Size = 100;
    constexpr size_t Count = 1000;
    const std::vector<int> items = utils_cpp::generate<std::vector>(Size, [cnt = 0]() mutable { return ++cnt; });

    while (state.KeepRunning()) {
        (void)utils_cpp::random_weighted_items(items, Count);
    }
}

BENCHMARK(benchmark_random_weighted_items_100);


static void benchmark_random_weighted_items_1000(benchmark::State& state)
{
    constexpr size_t Size = 1000;
    constexpr size_t Count = 1000;
    const std::vector<int> items = utils_cpp::generate<std::vector>(Size, [cnt = 0]() mutable { return ++cnt; });

    while (state.KeepRunning()) {
        (void)utils_cpp::random_weighted_items(items, Count);
    }
}

BENCHMARK(benchmark_random_weighted_items_1000);


static void benchmark_random_weighted_items_10000(benchmark::State& state)
{
    constexpr size_t Size = 10000;
    constexpr size_t Count = 1000;
    const std::vector<int> items = utils_cpp::generate<std::vector>(Size, [cnt = 0]() mutable { return ++cnt; });

    while (state.KeepRunning()) {
        (void)utils_cpp::random_weighted_items(items, Count);
    }
}

BENCHMARK(benchmark_random_weighted_items_10000);


BENCHMARK_MAIN();
