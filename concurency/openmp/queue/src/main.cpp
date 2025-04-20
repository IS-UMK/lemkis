#include <chrono>
#include <print>
#include <thread>

#include "include/omp_queue.hpp"

int main() {
    constexpr int NUM_PRODUCERS = 2;
    constexpr int NUM_CONSUMERS = 2;
    constexpr int ITEMS_PER_PRODUCER = 10;

    OMPQueue<int> queue;

#pragma omp parallel num_threads(NUM_PRODUCERS + NUM_CONSUMERS)
    {
        int tid = omp_get_thread_num();

        if (tid < NUM_PRODUCERS) {
            // Producer
            for (int i = 0; i < ITEMS_PER_PRODUCER; ++i) {
                int item = tid * 100 + i;
                queue.push(item);
#pragma omp critical
                std::println("Producer {} pushed {}", tid, item);
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        } else {
            // Consumer
            int item;
            while (true) {
                if (queue.pop(item)) {
#pragma omp critical
                    std::println("Consumer {} popped {}", tid, item);
                } else {
                    if (queue.empty()) break;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
            }
        }
    }

#pragma omp critical
    std::println("Done. Final size: {}", queue.size());

    return 0;
}
