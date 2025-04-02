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
    auto benchmark_concurrent_stack(int N) -> void;

    auto benchmark_concurrent_queue(int N) -> void;

    auto benchmark_stack(int N) -> void;

    auto benchmark_queue(int N) -> void;

    auto run_benchmarks(int N) -> void;
}  