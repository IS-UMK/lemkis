#include <omp.h>

#include <iostream>
#include <thread>

#include "./include/benchmark_utils.hpp"
#include "./include/own_test.hpp"


constexpr int k_default_threads = 4;

auto run_tests() -> void {
    measure_and_print("[omp_queue] OpenMP Benchmark", [] {
        return omp_queue_test(k_default_threads, k_default_threads);
    });
    measure_and_print("[concurrent_queue] jthread Benchmark", [] {
        return concurrent_queue_test(k_default_threads, k_default_threads);
    });
    run_own_test();
}


auto main() noexcept -> int {
    try {
        run_tests();
    } catch (const std::exception& e) {
        std::cerr << "Unhandled exception: " << e.what() << '\n';
    }
    return 0;
}
