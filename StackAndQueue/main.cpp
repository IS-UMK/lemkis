#include "src/include/benchmark.hpp"
#include "src/include/queue_test.hpp"
#include "src/include/stack_test.hpp"

auto main() -> int {
    const int benchmark_count = 100000;

    // QueueTest::queueTest();
    StackTest::stackTest();
    Benchmark::run_benchmarks(benchmark_count);
    Benchmark::run_benchmarks_new(benchmark_count);

    return 0;
}