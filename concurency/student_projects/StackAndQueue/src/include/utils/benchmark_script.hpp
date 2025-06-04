#pragma once

#include <list_stack.hpp>
#include <lock_free_queue_benchmark.hpp>
#include <memory>
#include <print>
#include <queue_cv_benchmark.hpp>
#include <queue_mutex_benchmark.hpp>
#include <reader_writer_queue_benchmark.hpp>
#include <stack_cv_benchmark.hpp>
#include <stack_mutex_benchmark.hpp>
#include <timer.hpp>
#include <vector>
#include <vector_stack.hpp>

namespace benchmark_script {
    using vector_stack_t = vector_stack<int>;
    using list_stack_t = list_stack<int>;

    static constexpr int single_producer = 1;
    static constexpr int single_consumer = 1;

    inline auto add_stack_benchmarks(
        std::vector<std::unique_ptr<benchmark_base>>& b,
        int p,
        int c,
        int n) -> void {
        b.emplace_back(std::make_unique<stack_mutex_benchmark<vector_stack_t>>(
            "vector_stack (mutex)", p, c, n));
        b.emplace_back(std::make_unique<stack_mutex_benchmark<list_stack_t>>(
            "list_stack (mutex)", p, c, n));
        b.emplace_back(std::make_unique<stack_cv_benchmark<vector_stack_t>>(
            "vector_stack (cv)", p, c, n));
        b.emplace_back(std::make_unique<stack_cv_benchmark<list_stack_t>>(
            "list_stack (cv)", p, c, n));
    }

    inline auto add_queue_benchmarks(
        std::vector<std::unique_ptr<benchmark_base>>& b,
        int p,
        int c,
        int n) -> void {
        b.emplace_back(std::make_unique<queue_mutex_benchmark>(
            "two_stack_queue (mutex)", p, c, n));
        b.emplace_back(std::make_unique<queue_cv_benchmark>(
            "two_stack_queue (cv)", p, c, n));
    }

    inline auto add_lockfree_benchmarks(
        std::vector<std::unique_ptr<benchmark_base>>& b,
        int p,
        int c,
        int n) -> void {
        b.emplace_back(std::make_unique<lock_free_queue_benchmark>(
            "moodycamel::ConcurrentQueue", p, c, n));
        if (p == single_producer && c == single_consumer) {
            b.emplace_back(std::make_unique<reader_writer_queue_benchmark>(
                "moodycamel::ReaderWriterQueue", n));
        }
    }

    inline auto create_all_benchmarks(int p, int c, int n)
        -> std::vector<std::unique_ptr<benchmark_base>> {
        std::vector<std::unique_ptr<benchmark_base>> b;
        add_stack_benchmarks(b, p, c, n);
        add_queue_benchmarks(b, p, c, n);
        add_lockfree_benchmarks(b, p, c, n);
        return b;
    }

    inline auto run_and_report(std::vector<std::unique_ptr<benchmark_base>> b)
        -> void {
        for (auto& bench : b) {
            timer t;
            bench->prepare_threads();
            t.start();
            bench->run();
            bench->print_result(t.elapsed());
        }
    }

    inline auto run_for_config(int p, int c, int n) -> void {
        std::print("{} producer(s), {} consumer(s):\n", p, c);
        auto benchmarks = create_all_benchmarks(p, c, n);
        run_and_report(std::move(benchmarks));
        std::print("\n");
    }

    inline auto run_all_benchmarks(const std::vector<int>& prod,
                                   const std::vector<int>& cons,
                                   int total) -> void {
        std::print("Running all benchmarks:\n");
        std::print("========================\n\n");
        for (const int p : prod) {
            for (const int c : cons) { run_for_config(p, c, total); }
        }
    }
}  // namespace benchmark_script