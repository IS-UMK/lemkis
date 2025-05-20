#include "include/old_benchmark.hpp"

#include <atomic>
#include <chrono>
#include <mutex>
#include <print>
#include <thread>
#include <vector>

#include "include/con_queue.hpp"
#include "include/con_stack.hpp"
#include "include/queue.hpp"
#include "include/stack.hpp"

namespace old_benchmark {
    template <typename Func>
    auto measure_time(Func&& func) -> int {
        auto start = std::chrono::high_resolution_clock::now();
        func();
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::milliseconds>(end -
                                                                     start);
    }

    auto old_benchmark::join_threads(auto& threads) -> void {
        for (auto& thread : threads) { thread.join(); }
    }

    template <typename StructType>
    auto run_producers(StructType& s,
                       std::atomic<int>& produced_count,
                       int n_producers,
                       int total_items,
                       std::vector<std::jthread>& producers) -> void {
        for (int p = 0; p < n_producers; ++p) {
            producers.emplace_back([&](const std::stop_token& stoken) {
                while (produced_count < total_items &&
                       !stoken.stop_requested()) {
                    s.push(produced_count);
                    produced_count++;
                }
            });
        }
    }

    template <typename StructType>
    auto run_consumers(StructType& s,
                       std::atomic<int>& consumed_count,
                       int n_consumers,
                       int total_items,
                       std::vector<std::jthread>& consumers) -> void {
        for (int p = 0; p < n_consumers; ++p) {
            consumers.emplace_back([&](const std::stop_token& stoken) {
                while (consumed_count < total_items &&
                       !stoken.stop_requested()) {
                    if (!s.empty()) {
                        s.pop();
                        consumed_count++;
                    }
                }
            });
        }
    }

    auto print_results(const std::string& name,
                       int n_producers,
                       int n_consumers,
                       int total_items,
                       auto duration) -> void {
        std::print(
            "{}: {} producers, {} consumers, {} items total - completed in {} "
            "ms\n",
            name,
            n_producers,
            n_consumers,
            total_items,
            duration.count());
    }

    template <typename StructType>
    auto benchmark_old_concurrent_structure(const std::string& name,
                                            int n_producers,
                                            int n_consumers,
                                            int total_items) -> void {
        StructType s;
        std::atomic<int> produced_count(0);
        std::atomic<int> consumed_count(0);
        std::vector<std::jthread> producers;
        std::vector<std::jthread> consumers;

        int time = measure_time([&] {
            run_producers(
                s, produced_count, n_producers, total_items, producers);
            run_consumers(
                s, consumed_count, n_consumers, total_items, consumers);
            join_threads(producers);
            join_threads(consumers);
        });

        print_results(name, n_producers, n_consumers, total_items, time);
    }

    template <typename StructType>
    auto benchmark_old_structure(const std::string& name,
                                 int n_producers,
                                 int n_consumers,
                                 int total_items) -> void {
        StructType s;
        std::atomic<int> push_count(0);
        std::atomic<int> pop_count(0);

        int time = measure_time([&] {
            while (push_count < total_items) {
                s.push(push_count);
                push_count++;
            }
            while (pop_count < total_items) {
                s.pop();
                pop_count++;
            }
        });

        print_results(name, n_producers, n_consumers, total_items, time);
    }

    auto run_old_benchmarks(int item_count) -> void {
        for (int const p : {1, 2, 4}) {
            for (int const c : {1, 2, 4}) {
                benchmark_old_concurrent_structure<concurrent_queue<int>>(
                    "Old concurrent queue", p, c, item_count);
                benchmark_old_concurrent_structure<concurrent_stack<int>>(
                    "Old concurrent queue", p, c, item_count);
            }
        }

        benchmark_old_structure<queue<int>>(
            "Old concurrent queue", 1, 1, item_count);
        benchmark_old_structure<stack<int>>(
            "Old concurrent queue", 1, 1, item_count);
    }
}  // namespace old_benchmark