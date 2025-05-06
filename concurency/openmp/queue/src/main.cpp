#include "include/own_test.hpp"
#include "include/queue_test.hpp"
#include <vector>
#include <print>

int main() {

    // Common thread configurations
    std::vector<std::pair<int, int>> configs = {
        {1, 1},  // SPSC - Single Producer, Single Consumer
        {1, 4},  // SPMC - Single Producer, Multiple Consumers
        {4, 1},  // MPSC - Multiple Producers, Single Consumer
        {2, 2},  // Balanced small
        {4, 4}   // Balanced medium
    };

    for (const auto& [producers, consumers] : configs) {
        run_full_comparison(producers, consumers);
    }

    //run_own_test();

    return 0;
}
