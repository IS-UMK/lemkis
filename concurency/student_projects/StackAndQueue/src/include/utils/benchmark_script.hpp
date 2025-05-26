#pragma once

#include <memory>
#include <print>
#include <vector>

#include "list_stack.hpp"
#include "lock_free_queue_benchmark.hpp"
#include "queue_cv_benchmark.hpp"
#include "queue_mutex_benchmark.hpp"
#include "reader_writer_queue_benchmark.hpp"
#include "stack_cv_benchmark.hpp"
#include "stack_mutex_benchmark.hpp"
#include "vector_stack.hpp"

namespace benchmark_script {

    using vector_stack_t = vector_stack<int>;
    using list_stack_t = list_stack<int>;

    constexpr int single_producer = 1;
    constexpr int single_consumer = 1;

    inline auto add_stack_benchmarks(
        std::vector<std::unique_ptr<BenchmarkBase>>& b,
        int p,
        int c,
        int n) -> void {
        b.emplace_back(std::make_unique<StackMutexBenchmark<vector_stack_t>>(
            "vector_stack (mutex)", p, c, n));
        b.emplace_back(std::make_unique<StackMutexBenchmark<list_stack_t>>(
            "list_stack (mutex)", p, c, n));
        b.emplace_back(std::make_unique<StackCVBenchmark<vector_stack_t>>(
            "vector_stack (cv)", p, c, n));
        b.emplace_back(std::make_unique<StackCVBenchmark<list_stack_t>>(
            "list_stack (cv)", p, c, n));
    }

    inline auto add_queue_benchmarks(
        std::vector<std::unique_ptr<BenchmarkBase>>& b,
        int p,
        int c,
        int n) -> void {
        b.emplace_back(std::make_unique<QueueMutexBenchmark>(
            "two_stack_queue (mutex)", p, c, n));
        b.emplace_back(std::make_unique<QueueCVBenchmark>(
            "two_stack_queue (cv)", p, c, n));
    }

    inline auto add_lockfree_benchmarks(
        std::vector<std::unique_ptr<BenchmarkBase>>& b,
        int p,
        int c,
        int n) -> void {
        b.emplace_back(std::make_unique<LockFreeQueueBenchmark>(
            "moodycamel::ConcurrentQueue", p, c, n));
        if (p == single_producer && c == single_consumer) {
            b.emplace_back(std::make_unique<ReaderWriterQueueBenchmark>(
                "moodycamel::ReaderWriterQueue", n));
        }
    }

    inline auto run_for_config(int p, int c, int n) -> void {
        std::print("{} producer(s), {} consumer(s):\n", p, c);
        std::vector<std::unique_ptr<BenchmarkBase>> b;
        add_stack_benchmarks(b, p, c, n);
        add_queue_benchmarks(b, p, c, n);
        add_lockfree_benchmarks(b, p, c, n);
        for (auto& bench : b) { bench->run(); }
        std::print("\n");
    }

    inline auto run_all_benchmarks(const std::vector<int>& prod,
                                   const std::vector<int>& cons,
                                   int total) -> void {
        std::print("Running all benchmarks:\n");
        std::print("========================\n\n");
        for (int p : prod) {
            for (int c : cons) { run_for_config(p, c, total); }
        }
    }
}  // namespace benchmark_script