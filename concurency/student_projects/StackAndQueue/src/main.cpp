#include <cstdio>
#include <exception>
#include <print>

#include "include/utils/benchmark_script.hpp"

constexpr int benchmark_count = 30000;
constexpr int zero = 0;
constexpr int one = 1;

namespace {
    inline auto run_all_configurations() -> void {
        const std::vector<int> producer_counts{1, 2, 4};
        const std::vector<int> consumer_counts{1, 2, 4};
        benchmark_script::run_all_benchmarks(
            producer_counts, consumer_counts, benchmark_count);
    }
}  // namespace

auto main() noexcept -> int {
    try {
        run_all_configurations();
        return zero;
    } catch (const std::exception& e) {
        std::fputs("Unhandled std::exception: ", stdout);
        std::fputs(e.what(), stdout);
        std::fputs("\n", stdout);
    } catch (...) { std::fputs("Unhandled unknown exception\n", stdout); }
    return one;
}
