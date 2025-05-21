#pragma once

namespace benchmark {
    auto benchmark_concurrent_stack(int N) -> void;

    auto benchmark_concurrent_queue(int N) -> void;

    auto benchmark_stack(int N) -> void;

    auto benchmark_queue(int N) -> void;

    auto run_old_benchmarks(int N) -> void;

    auto run_new_benchmarks(int N) -> void;
}  // namespace benchmark