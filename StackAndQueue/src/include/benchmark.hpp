#pragma once

namespace Benchmark {
    auto benchmark_concurrent_stack(int N) -> void;

    auto benchmark_concurrent_queue(int N) -> void;

    auto benchmark_stack(int N) -> void;

    auto benchmark_queue(int N) -> void;

    auto run_benchmarks(int N) -> void;

    auto run_benchmarks_new(int N) -> void;
}  // namespace Benchmark