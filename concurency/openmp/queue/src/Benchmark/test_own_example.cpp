// benchmark_std_vs_omp.cpp
#include <algorithm>
#include <execution>
#include <vector>
#include <numeric>
#include <iostream>
#include <random>
#include <chrono>
#include <omp.h>

using namespace std;

constexpr size_t SMALL_SIZE = 10'000;
constexpr size_t LARGE_SIZE = 100'000'000;

vector<double> generate_random_vector(size_t size) {
    vector<double> vec(size);
    mt19937 gen(42);
    uniform_real_distribution<> dis(0.0, 1.0);
    generate(vec.begin(), vec.end(), [&]() { return dis(gen); });
    return vec;
}

template <typename Func>
void benchmark(const string& name, Func func) {
    auto start = chrono::high_resolution_clock::now();
    func();
    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> diff = end - start;
    cout << name << " time: " << diff.count() << " s\n";
}

void test_transform(const vector<double>& input, vector<double>& output) {
    benchmark("std::transform (seq)", [&]() {
        transform(execution::seq, input.begin(), input.end(), output.begin(), [](double x) { return x * x; });
    });

    benchmark("std::transform (par)", [&]() {
        transform(execution::par, input.begin(), input.end(), output.begin(), [](double x) { return x * x; });
    });

    benchmark("std::transform (par_unseq)", [&]() {
        transform(execution::par_unseq, input.begin(), input.end(), output.begin(), [](double x) { return x * x; });
    });

    benchmark("OpenMP transform", [&]() {
        #pragma omp parallel for
        for (int i = 0; i < static_cast<int>(input.size()); ++i) {
            output[i] = input[i] * input[i];
        }
    });
}

void test_dot_product(const vector<double>& a, const vector<double>& b) {
    benchmark("std::inner_product (seq)", [&]() {
        volatile double res = inner_product(a.begin(), a.end(), b.begin(), 0.0);
    });

    benchmark("std::transform_reduce (par)", [&]() {
        volatile double res = transform_reduce(execution::par, a.begin(), a.end(), b.begin(), 0.0);
    });

    benchmark("std::transform_reduce (par_unseq)", [&]() {
        volatile double res = transform_reduce(execution::par_unseq, a.begin(), a.end(), b.begin(), 0.0);
    });

    benchmark("OpenMP dot product", [&]() {
        double res = 0.0;
        #pragma omp parallel for reduction(+:res)
        for (int i = 0; i < static_cast<int>(a.size()); ++i) {
            res += a[i] * b[i];
        }
        volatile double sink = res;
    });
}

int main() {
    cout << "==== SMALL DATASET ====" << endl;
    auto a_small = generate_random_vector(SMALL_SIZE);
    auto b_small = generate_random_vector(SMALL_SIZE);
    vector<double> output_small(SMALL_SIZE);

    cout << "\n= Testing Transform Product =" << endl;
    test_transform(a_small, output_small);

    cout << "\n= Testing DOT Product =" << endl;
    test_dot_product(a_small, b_small);

    cout << "\n==== LARGE DATASET ====" << endl;
    auto a_large = generate_random_vector(LARGE_SIZE);
    auto b_large = generate_random_vector(LARGE_SIZE);
    vector<double> output_large(LARGE_SIZE);

    cout << "\n= Testing Transform Product =" << endl;
    test_transform(a_large, output_large);

    cout << "\n= Testing DOT Product =" << endl;
    test_dot_product(a_large, b_large);

    return 0;
}
