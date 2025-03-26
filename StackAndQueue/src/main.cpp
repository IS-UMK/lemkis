#include "queue_test.hpp"
#include "stack_test.hpp"
#include "benchmark.hpp"

auto main() -> int {
    //QueueTest::queueTest();
    //StackTest::stackTest();
    Benchmark::run_benchmarks(1000);
    return 0;
}