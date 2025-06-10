#include <benchmark_script.hpp>
#include <cstdio>
#include <exception>
#include <string_view>

namespace {
    constexpr std::string_view file_name = "results.csv";
}

auto main() noexcept -> int {
    constexpr int success_code = 0;
    constexpr int error_code = 1;
    
    try {
        benchmark_script::run_all_configurations(file_name);
        return success_code;
    } catch (const std::exception& e) {
        std::fputs("Unhandled std::exception: ", stdout);
        std::fputs(e.what(), stdout);
        std::fputs("\n", stdout);
    } catch (...) { 
        std::fputs("Unhandled unknown exception\n", stdout); 
    }
    return error_code;
}
