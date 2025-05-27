#include <omp.h>

#include <algorithm>
#include <chrono>
#include <execution>
#include <format>
#include <iostream>
#include <numeric>
#include <print>
#include <random>

#include "../include/own_test.hpp"

// Named constants instead of magic numbers
constexpr int RANDOM_SEED = 42;
constexpr double DISTRIBUTION_MIN = 0.0;
constexpr double DISTRIBUTION_MAX = 1.0;

auto generate_random_vector(std::size_t size) -> std::vector<double> {
    std::vector<double> vec(size);
    std::mt19937 gen(RANDOM_SEED);
    std::uniform_real_distribution<> dis(DISTRIBUTION_MIN, DISTRIBUTION_MAX);
    std::ranges::generate(vec, [&]() { return dis(gen); });
    return vec;
}

template <typename Func>
auto benchmark(const std::string& name, Func func) -> void {
    auto start = std::chrono::high_resolution_clock::now();
    func();
    auto end = std::chrono::high_resolution_clock::now();
    const std::chrono::duration<double> diff = end - start;
    std::print("{} time: {} s\n", name, diff.count());
}

// Sequential transform benchmark
auto benchmark_transform_seq(const std::vector<double>& input, std::vector<double>& output) -> void {
    benchmark("std::transform (seq)", [&]() {
        std::transform(std::execution::seq,
                       input.begin(),
                       input.end(),
                       output.begin(),
                       [](double x) { return x * x; });
    });
}

// Parallel transform benchmark
auto benchmark_transform_par(const std::vector<double>& input, std::vector<double>& output) -> void {
    benchmark("std::transform (par)", [&]() {
        std::transform(std::execution::par,
                       input.begin(),
                       input.end(),
                       output.begin(),
                       [](double x) { return x * x; });
    });
}

// Parallel unsequenced transform benchmark
auto benchmark_transform_par_unseq(const std::vector<double>& input, std::vector<double>& output) -> void {
    benchmark("std::transform (par_unseq)", [&]() {
        std::transform(std::execution::par_unseq,
                       input.begin(),
                       input.end(),
                       output.begin(),
                       [](double x) { return x * x; });
    });
}

// OpenMP transform benchmark
auto benchmark_transform_openmp(const std::vector<double>& input, std::vector<double>& output) -> void {
    benchmark("OpenMP transform", [&]() {
#pragma omp parallel for
        for (int i = 0; i < static_cast<int>(input.size()); ++i) {
            output[i] = input[i] * input[i];
        }
    });
}

auto test_transform(const std::vector<double>& input, std::vector<double>& output) -> void {
    benchmark_transform_seq(input, output);
    benchmark_transform_par(input, output);
    benchmark_transform_par_unseq(input, output);
    benchmark_transform_openmp(input, output);
}

// Sequential dot product benchmark
auto benchmark_dot_product_seq(const std::vector<double>& a, const std::vector<double>& b) -> void {
    benchmark("std::inner_product (seq)", [&]() {
        double result = std::inner_product(a.begin(), a.end(), b.begin(), 0.0);
        std::print("Result: {}\n", result);
    });
}

// Parallel dot product benchmark
auto benchmark_dot_product_par(const std::vector<double>& a, const std::vector<double>& b) -> void {
    benchmark("std::transform_reduce (par)", [&]() {
        double result = std::transform_reduce(
            std::execution::par, a.begin(), a.end(), b.begin(), 0.0);
        std::print("Result: {}\n", result);
    });
}

// Parallel unsequenced dot product benchmark
auto benchmark_dot_product_par_unseq(const std::vector<double>& a, const std::vector<double>& b) -> void {
    benchmark("std::transform_reduce (par_unseq)", [&]() {
        double result = std::transform_reduce(
            std::execution::par_unseq, a.begin(), a.end(), b.begin(), 0.0);
        std::print("Result: {}\n", result);
    });
}

// OpenMP dot product benchmark
auto benchmark_dot_product_openmp(const std::vector<double>& a, const std::vector<double>& b) -> void {
    benchmark("OpenMP dot product", [&]() {
        double res = 0.0;
#pragma omp parallel for reduction(+ : res)
        for (int i = 0; i < static_cast<int>(a.size()); ++i) {
            res += a[i] * b[i];
        }
        std::print("Result: {}\n", res);
    });
}

auto test_dot_product(const std::vector<double>& a, const std::vector<double>& b) -> void {
    benchmark_dot_product_seq(a, b);
    benchmark_dot_product_par(a, b);
    benchmark_dot_product_par_unseq(a, b);
    benchmark_dot_product_openmp(a, b);
}

// Test small dataset
auto run_small_dataset_tests() -> void {
    std::print("==== SMALL DATASET ====\n");
    auto a_small = generate_random_vector(small_size);
    auto b_small = generate_random_vector(small_size);
    std::vector<double> output_small(small_size);

    std::print("\n= Testing Transform Product =\n");
    test_transform(a_small, output_small);

    std::print("\n= Testing DOT Product =\n");
    test_dot_product(a_small, b_small);
}

// Test large dataset
auto run_large_dataset_tests() -> void {
    std::print("\n==== LARGE DATASET ====\n");
    auto a_large = generate_random_vector(large_size);
    auto b_large = generate_random_vector(large_size);
    std::vector<double> output_large(large_size);

    std::print("\n= Testing Transform Product =\n");
    test_transform(a_large, output_large);

    std::print("\n= Testing DOT Product =\n");
    test_dot_product(a_large, b_large);
}

auto run_own_test() -> void {
    run_small_dataset_tests();
    run_large_dataset_tests();
}