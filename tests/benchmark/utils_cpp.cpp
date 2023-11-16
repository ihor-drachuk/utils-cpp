/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/utils-cpp
 * Contact:  ihor-drachuk-libs@pm.me  */

#include <benchmark/benchmark.h>
#include <utils-cpp/lazyinit.h>

static void StubBenchmark(benchmark::State& state)
{
    int i = 0;

    while (state.KeepRunning())
        i++;

    (void)i;
}

BENCHMARK(StubBenchmark);


static void LazyInitBenchmark(benchmark::State& state)
{
    while (state.KeepRunning()) {
        LazyInit<std::string> li = [](){return new std::string();};
        (void)li;
    }
}

BENCHMARK(LazyInitBenchmark);


static void LazyInitBenchmark_Off(benchmark::State& state)
{
    while (state.KeepRunning()) {
        std::string str;
        (void)str;
    }
}

BENCHMARK(LazyInitBenchmark_Off);


BENCHMARK_MAIN();
