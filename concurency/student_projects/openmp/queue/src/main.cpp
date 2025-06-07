#include <omp.h>

#include <benchmark_utils.hpp>
#include <iostream>
#include <own_test.hpp>
#include <thread>


constexpr int k_default_threads = 4;
constexpr int error_code = 1;
namespace {
    auto run_tests() -> void {
        measure_and_print("[omp_queue] OpenMP Benchmark", [] {
            return omp_bench::omp_queue_test(k_default_threads,
                                             k_default_threads);
        });
        measure_and_print("[concurrent_queue] jthread Benchmark", [] {
            return omp_bench::concurrent_queue_test(k_default_threads,
                                                    k_default_threads);
        });
        own_bench::run_own_test();
    }
}  // namespace
// NOLINTNEXTLINE(misc-use-internal-linkage)
auto main() noexcept -> int {

    try {
        run_tests();
    } catch (const std::exception& e) {
        std::cerr << "Unhandled exception: " << e.what() << '\n';
        return error_code;
    }
    return 0;
}
