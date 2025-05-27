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


auto generate_random_vector(std::size_t size) -> std::vector<double> {
    std::vector<double> vec(size);
    std::mt19937 gen(42);
    std::uniform_real_distribution<> dis(0.0, 1.0);
    std::generate(vec.begin(), vec.end(), [&]() { return dis(gen); });
    return vec;
}

template <typename Func>
auto benchmark(const std::string& name, Func func) -> void {
    auto start = std::chrono::high_resolution_clock::now();
    func();
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end - start;
    std::print("{} time: {} s\n", name, diff.count());
}

auto test_transform(const std::vector<double>& input,
                    std::vector<double>& output) -> void {
    benchmark("std::transform (seq)", [&]() {
        std::transform(std::execution::seq,
                       input.begin(),
                       input.end(),
                       output.begin(),
                       [](double x) { return x * x; });
    });

    benchmark("std::transform (par)", [&]() {
        std::transform(std::execution::par,
                       input.begin(),
                       input.end(),
                       output.begin(),
                       [](double x) { return x * x; });
    });

    benchmark("std::transform (par_unseq)", [&]() {
        std::transform(std::execution::par_unseq,
                       input.begin(),
                       input.end(),
                       output.begin(),
                       [](double x) { return x * x; });
    });

    benchmark("OpenMP transform", [&]() {
#pragma omp parallel for
        for (int i = 0; i < static_cast<int>(input.size()); ++i) {
            output[i] = input[i] * input[i];
        }
    });
}

auto test_dot_product(const std::vector<double>& a,
                      const std::vector<double>& b) -> void {
    benchmark("std::inner_product (seq)", [&]() {
        double result = std::inner_product(a.begin(), a.end(), b.begin(), 0.0);
        std::print("Result: {}\n", result);
    });

    benchmark("std::transform_reduce (par)", [&]() {
        double result = std::transform_reduce(
            std::execution::par, a.begin(), a.end(), b.begin(), 0.0);
        std::print("Result: {}\n", result);
    });

    benchmark("std::transform_reduce (par_unseq)", [&]() {
        double result = std::transform_reduce(
            std::execution::par_unseq, a.begin(), a.end(), b.begin(), 0.0);
        std::print("Result: {}\n", result);
    });

    benchmark("OpenMP dot product", [&]() {
        double res = 0.0;
#pragma omp parallel for reduction(+ : res)
        for (int i = 0; i < static_cast<int>(a.size()); ++i) {
            res += a[i] * b[i];
        }
        std::print("Result: {}\n", res);
    });
}


auto run_own_test() -> void {
    std::print("==== SMALL DATASET ====\n");
    auto a_small = generate_random_vector(SMALL_SIZE);
    auto b_small = generate_random_vector(SMALL_SIZE);
    std::vector<double> output_small(SMALL_SIZE);

    std::print("\n= Testing Transform Product =\n");
    test_transform(a_small, output_small);

    std::print("\n= Testing DOT Product =\n");
    test_dot_product(a_small, b_small);

    std::print("\n==== LARGE DATASET ====\n");
    auto a_large = generate_random_vector(LARGE_SIZE);
    auto b_large = generate_random_vector(LARGE_SIZE);
    std::vector<double> output_large(LARGE_SIZE);

    std::print("\n= Testing Transform Product =\n");
    test_transform(a_large, output_large);

    std::print("\n= Testing DOT Product =\n");
    test_dot_product(a_large, b_large);
}
