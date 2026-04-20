#pragma once

#include <chrono>
#include <format>
#include <print>
#include <string>

// Measures execution time of a function returning (pushed, popped, remaining),
// and prints the results along with a label.
// The function must return a std::tuple<int, int, std::size_t>.
template <typename Func>
auto measure_and_print(const std::string& label, Func&& func) -> void {
    const auto start = std::chrono::high_resolution_clock::now();
    auto [pushed, popped, remaining] = func();
    const auto end = std::chrono::high_resolution_clock::now();
    double time_seconds = std::chrono::duration<double>(end - start).count();
    std::print("\n{}\nRemaining: {}, Time: {:.4f} s\n",
               label,
               remaining,
               time_seconds);
}

// Namespace containing concurrent queue benchmark functions using different
// strategies.
namespace omp_bench {

    // Runs a benchmark using a thread-safe concurrent queue with multiple
    // producers and consumers. Returns a tuple: (number of items pushed,
    // popped, and remaining in the queue).
    auto concurrent_queue_test(int producers, int consumers)
        -> std::tuple<int, int, std::size_t>;

    // Runs a benchmark using an OpenMP-based queue strategy with multiple
    // producers and consumers. Returns a tuple: (number of items pushed,
    // popped, and remaining in the queue).
    auto omp_queue_test(int producers, int consumers)
        -> std::tuple<int, int, std::size_t>;

}  // namespace omp_bench
