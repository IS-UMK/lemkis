#include "include/benchmark.hpp"
#include "include/queue_test.hpp"
#include "include/stack_test.hpp"

auto main() -> int {
    const int benchmark_count = 100000;

    queue_test::run_queue_test();
    stack_test::run_stack_test();
    benchmark::run_benchmarks(benchmark_count);
    benchmark::run_benchmarks_new(benchmark_count);

    return 0;
}