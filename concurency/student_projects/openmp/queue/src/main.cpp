#include <iostream>
#include <print>
#include <vector>

#include "include/own_test.hpp"
#include "include/queue_test.hpp"

// Constants for benchmark configurations
constexpr int single_thread = 1;
constexpr int small_thread_count = 2;
constexpr int medium_thread_count = 4;
constexpr int error_code = 1;

// Anonymous namespace for internal linkage
namespace {
    // Get benchmark configurations
    auto get_benchmark_configs() -> std::vector<std::pair<int, int>> {
        return {{single_thread, single_thread},
                {single_thread, medium_thread_count},
                {medium_thread_count, single_thread},
                {small_thread_count, small_thread_count},
                {medium_thread_count, medium_thread_count}};
    }

    // Run benchmarks for all configurations
    auto run_queue_benchmarks() -> void {
        auto const configs = get_benchmark_configs();
        for (const auto& [producers, consumers] : configs) {
            run_full_comparison(producers, consumers);
        }
    }
}  // namespace

auto main() -> int {
    try {
        run_queue_benchmarks();
        run_own_test();
    } catch (const std::exception& e) {
        std::cerr << "Błąd: " << e.what();
        return error_code;
    }
    return 0;
}
