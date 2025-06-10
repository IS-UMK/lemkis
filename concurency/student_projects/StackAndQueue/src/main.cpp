#include <benchmark_script.hpp>
#include <codes.hpp>
#include <cstdio>
#include <exception>
#include <string_view>

namespace {
    constexpr std::string_view file_name = "results.csv";
}  // namespace

auto main() noexcept -> int {
    try {
        benchmark_script::run_all_configurations(file_name);
        return static_cast<int>(codes::success);
    } catch (const std::exception& e) {
        std::fputs("Unhandled std::exception: ", stdout);
        std::fputs(e.what(), stdout);
        std::fputs("\n", stdout);
    } catch (...) { std::fputs("Unhandled unknown exception\n", stdout); }
    return static_cast<int>(codes::error);
}
