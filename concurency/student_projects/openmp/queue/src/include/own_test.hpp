#include <string>
#include <vector>

// Constants for dataset sizes used in performance benchmarks
constexpr size_t small_size = 10'000;       // Small dataset for quick tests
constexpr size_t large_size = 300'000'000;  // Large dataset for stress tests

// Namespace for custom benchmarking functions
namespace own_bench {
    // Runs all custom test suites with predefined dataset sizes
    auto run_own_test() -> void;
}  // namespace own_bench

// Generates a vector of random double values in range [0.0, 1.0]
auto generate_random_vector(size_t size) -> std::vector<double>;

// Measures and prints the execution time of the provided function
template <typename Func>
void benchmark(const std::string& name, Func func);

// Runs benchmarking for various parallel and sequential transform operations
void test_transform(const std::vector<double>& input,
                    std::vector<double>& output);

// Runs benchmarking for various dot product implementations
void test_dot_product(const std::vector<double>& a,
                      const std::vector<double>& b);
