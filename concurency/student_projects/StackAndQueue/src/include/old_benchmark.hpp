#pragma once

#include <atomic>
#include <string>
#include <thread>
#include <vector>

namespace old_benchmark {
    template <typename Func>
    auto measure_time(Func&& func) -> int;
    auto join_threads(auto& threads) -> void;
    template <typename StructType>
    auto run_producers(StructType& s,
                       std::atomic<int>& produced_count,
                       int n_producers,
                       int total_items,
                       std::vector<std::jthread>& producers) -> void;
    template <typename StructType>
    auto run_consumers(StructType& s,
                       std::atomic<int>& consumed_count,
                       int n_consumers,
                       int total_items,
                       std::vector<std::jthread>& consumers) -> void;
    auto print_results(const std::string& name,
                       int n_producers,
                       int n_consumers,
                       int total_items,
                       auto duration) -> void;
    template <typename StructType>
    auto benchmark_old_concurrent_structure(const std::string& name,
                                            int n_producers,
                                            int n_consumers,
                                            int total_items) -> void;
    template <typename StructType>
    auto benchmark_old_structure(const std::string& name,
                                 int n_producers,
                                 int n_consumers,
                                 int total_items) -> void;
    auto run_old_benchmarks(int item_count) -> void;
}  // namespace old_benchmark