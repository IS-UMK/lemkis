#pragma once

#include <vector>

namespace benchmark_script {
    void run_all_benchmarks(const std::vector<int>& prod,
                            const std::vector<int>& cons,
                            int total);
}