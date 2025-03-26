#pragma once

#include <chrono>
#include <print>
#include <thread>
#include <vector>

#include "con_queue.hpp"
#include "con_stack.hpp"
#include "queue.hpp"
#include "stack.hpp"

namespace Benchmark {
    // Benchmark for ConcurrentStack
    auto benchmark_concurrent_stack(int N) -> void;

    // Benchmark for ConcurrentQueue
    auto benchmark_concurrent_queue(int N) -> void;

    // Benchmark for Stack (single-threaded)
    auto benchmark_stack(int N) -> void;

    // Benchmark for Queue (single-threaded)
    auto benchmark_queue(int N) -> void;

    // Run all benchmarks
    auto run_benchmarks(int N) -> void;
}  // namespace Benchmark