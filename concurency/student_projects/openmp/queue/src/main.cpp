#include <print>
#include <vector>

#include "include/own_test.hpp"
#include "include/queue_test.hpp"

// Constants for benchmark configurations
constexpr int single_thread = 1;
constexpr int small_thread_count = 2;
constexpr int medium_thread_count = 4;

// Get benchmark configurations
auto get_benchmark_configs() -> std::vector<std::pair<int, int>> {
    return {
        {single_thread, single_thread},  // SPSC - Single Producer, Single Consumer
        {single_thread, medium_thread_count},  // SPMC - Single Producer, Multiple Consumers
        {medium_thread_count, single_thread},  // MPSC - Multiple Producers, Single Consumer
        {small_thread_count, small_thread_count},  // Balanced small
        {medium_thread_count, medium_thread_count}   // Balanced medium
    };
}

// Run benchmarks for all configurations
auto run_queue_benchmarks() -> void {
    auto const configs = get_benchmark_configs();
    for (const auto& [producers, consumers] : configs) {
        run_full_comparison(producers, consumers);
    }
}

auto main() -> int {
    run_queue_benchmarks();
    run_own_test();
    return 0;
}