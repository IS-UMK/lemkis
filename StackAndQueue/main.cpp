#include "src/include/benchmark.hpp"
#include "src/include/queue_test.hpp"
#include "src/include/stack_test.hpp"

auto main() -> int {
    // QueueTest::queueTest();
    StackTest::stackTest();
    Benchmark::run_benchmarks(1000);
    return 0;
}