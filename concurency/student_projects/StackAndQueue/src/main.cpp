#include <vector>

#include "benchmark_script.hpp"

auto main() -> int {
    const std::vector<int> prod_counts = {1, 2, 4};
    const std::vector<int> cons_counts = {1, 2, 4};
    const int benchmark_count = 1000;

    benchmark_script::run_all_benchmarks(
        prod_counts, cons_counts, benchmark_count);

    return 0;
}