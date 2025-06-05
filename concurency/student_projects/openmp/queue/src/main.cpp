#include <omp.h>

#include <thread>

#include "./include/benchmark_utils.hpp"
#include "./include/own_test.hpp"

int main() {
    measure_and_print("[omp_queue] OpenMP Benchmark",
                      [] { return omp_queue_test(4, 4); });

    measure_and_print("[concurrent_queue] jthread Benchmark",
                      [] { return concurrent_queue_test(4, 4); });

    run_own_test();
}
