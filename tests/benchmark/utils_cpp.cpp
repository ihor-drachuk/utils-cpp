/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/utils-cpp
 * Contact:  ihor-drachuk-libs@pm.me  */

#include <benchmark/benchmark.h>
#include <utils-cpp/lazy_init.h>

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


BENCHMARK_MAIN();
