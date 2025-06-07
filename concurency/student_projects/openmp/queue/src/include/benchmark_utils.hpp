#pragma once
#include <chrono>
#include <format>
#include <print>
#include <string>


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

auto concurrent_queue_test(int producers, int consumers)
    -> std::tuple<int, int, std::size_t>;
auto omp_queue_test(int producers, int consumers)
    -> std::tuple<int, int, std::size_t>;