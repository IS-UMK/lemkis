#include "include/benchmark.hpp"
#include "include/old_benchmark.hpp"
#include "include/struct_test.hpp"

auto main() -> int {
    const int benchmark_count = 1000;

    struct_test::run_queue_test();
    struct_test::run_stack_test();
    old_benchmark::run_old_benchmarks(benchmark_count);
    benchmark::run_new_benchmarks(benchmark_count);

    return 0;
}