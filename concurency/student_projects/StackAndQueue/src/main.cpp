#pragma once

#include "include/benchmark.hpp"

auto main() -> int {
    const int benchmark_count = 1000;

    benchmark::run_new_benchmarks(benchmark_count);

    return 0;
}