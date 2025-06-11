#include <omp.h>

#include <benchmark_utils.hpp>
#include <iostream>
#include <thread>
#include <transform_dot_benchmark.hpp>

constexpr int k_default_threads =
    4;  // Default number of producer and consumer threads
constexpr int error_code = 1;  // Return code on error

namespace {
    // Runs all benchmark tests and prints the results
    // Benchmark using OpenMP-based queue implementation
    // Benchmark using concurrent_queue with std::jthread
    // Runs custom tests defined in own_test module
    auto run_tests() -> void {
        measure_and_print("[omp_queue] OpenMP Benchmark", [] {
            return omp_bench::omp_queue_test(k_default_threads,
                                             k_default_threads);
        });
        measure_and_print("[concurrent_queue] jthread Benchmark", [] {
            return omp_bench::concurrent_queue_test(k_default_threads,
                                                    k_default_threads);
        });
        own_bench::run_own_test();
    }
}  // namespace

// Program entry point
// Runs benchmarks and handles any exceptions, returning error code if needed
auto main() noexcept -> int {
    try {
        run_tests();
    } catch (const std::exception& e) {
        std::cerr << "Unhandled exception: " << e.what() << '\n';
        return error_code;
    }
    return 0;
}
