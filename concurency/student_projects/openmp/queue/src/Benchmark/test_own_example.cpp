#include <omp.h>

#include <algorithm>
#include <chrono>
#include <execution>
#include <iostream>
#include <numeric>
#include <random>
#include <vector>
#include "../include/own_test.hpp"
constexpr int random_seed = 42;
constexpr double distribution_min = 0.0;
constexpr double distribution_max = 1.0;
constexpr int thousand = 1000;
namespace {
    auto generate_random_vector(std::size_t size) -> std::vector<double> {
        std::vector<double> vec(size);
        std::mt19937 gen(random_seed);
        std::uniform_real_distribution<> dis(distribution_min,
                                             distribution_max);
        std::generate(vec.begin(), vec.end(), [&]() { return dis(gen); });
        return vec;
    }

    template <typename Func>
    void benchmark(const std::string& name, Func func) {
        auto start = std::chrono::high_resolution_clock::now();
        func();
        auto end = std::chrono::high_resolution_clock::now();
        std::cout << name << " time: "
                  << std::chrono::duration<double>(end - start).count()
                  << " s\n";
    }

    auto benchmark_transform_seq(const std::vector<double>& input,
                                 std::vector<double>& output) -> void {
        benchmark("std::transform (seq)", [&]() {
            std::transform(
                input.begin(), input.end(), output.begin(), [](double x) {
                    return x * x;
                });
        });
    }

    auto benchmark_transform_par(const std::vector<double>& input,
                                 std::vector<double>& output) -> void {
        benchmark("std::transform (par)", [&]() {
            std::transform(std::execution::par,
                           input.begin(),
                           input.end(),
                           output.begin(),
                           [](double x) { return x * x; });
        });
    }

    auto benchmark_transform_openmp(const std::vector<double>& input,
                                    std::vector<double>& output) -> void {
        benchmark("OpenMP transform", [&]() {
#pragma omp parallel for
            for (std::size_t i = 0; i < input.size(); ++i) {
                output[i] = input[i] * input[i];
            }
        });
    }

    void test_transform(const std::vector<double>& input,
                        std::vector<double>& output) {
        benchmark_transform_seq(input, output);
        benchmark_transform_par(input, output);
        benchmark_transform_openmp(input, output);
    }

    auto benchmark_dot_product_openmp(const std::vector<double>& a,
                                      const std::vector<double>& b) -> void {
        benchmark("OpenMP dot product", [&]() {
            double res = 0.0;
#pragma omp parallel for reduction(+ : res)
            for (std::size_t i = 0; i < a.size(); ++i) { res += a[i] * b[i]; }
            std::cout << "Result: " << res << "\n";
        });
    }

    void test_dot_product(const std::vector<double>& a,
                          const std::vector<double>& b) {
        benchmark_dot_product_openmp(a, b);
    }

    void run_tests(std::size_t data_size) {
        std::cout << "\n==== DATASET SIZE: " << data_size << " ====\n";
        auto a = generate_random_vector(data_size);
        auto b = generate_random_vector(data_size);
        std::vector<double> output(data_size);
        std::cout << "\n= Testing Transform Product =\n";
        test_transform(a, output);
        std::cout << "\n= Testing DOT Product =\n";
        test_dot_product(a, b);
    }

}  // namespace
void run_own_test() {
    run_tests(thousand);  // Example small dataset
    run_tests(static_cast<std::size_t>(thousand) *
              thousand);  // Example large dataset
}
