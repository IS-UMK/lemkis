// --------------------------------------------------------------
// Includes and Constants
// --------------------------------------------------------------

#include <benchmark_utils.hpp>
#include <concurrent_queue.hpp>
#include <thread>
#include <vector>

constexpr int item_limit = 1000000;  // Total number of items to be pushed
constexpr int id_offset = 10000;  // Base offset to differentiate producer IDs
constexpr int k_increment = 1;    // Increment value for counters

// --------------------------------------------------------------
// Internal State and Helpers (anonymous namespace)
// --------------------------------------------------------------

namespace {
    std::atomic<bool> done{false};    // Signals when pushing is completed
    concurrent_queue<int> queue_con;  // Thread-safe concurrent queue
    std::atomic<int> pushed_con;      // Counter for pushed items
    std::atomic<int> popped_con;      // Counter for popped items
    std::vector<std::jthread>
        threads;  // Thread container for producers/consumers

    // Pushes 'count' items starting from 'base' into the queue
    void push_items(int base, int count) {
        for (int j = 0; j < count; ++j) {
            queue_con.push(j + base);
            pushed_con.fetch_add(k_increment, std::memory_order_relaxed);
        }
        if (pushed_con.load() >= item_limit) { done.store(true); }
    }

    // Continuously attempts to pop items from the queue until all are consumed
    void consume_items() {
        while (true) {
            if (done.load() && queue_con.empty()) { break; }
            if (queue_con.try_pop()) {
                popped_con.fetch_add(k_increment, std::memory_order_relaxed);
            } else {
                std::this_thread::yield();  // Relinquish CPU if queue is empty
            }
        }
    }

    // Launches 'count' producer threads, each pushing a share of the total
    // items
    void launch_producers(int count) {
        int const per = item_limit / count;
        for (int i = 0; i < count; ++i) {
            threads.emplace_back(push_items, i * id_offset, per);
        }
    }

    // Launches 'count' consumer threads
    void launch_consumers(int count) {
        for (int i = 0; i < count; ++i) { threads.emplace_back(consume_items); }
    }
}  // namespace

// --------------------------------------------------------------
// Benchmark Interface (omp_bench namespace)
// --------------------------------------------------------------

namespace omp_bench {

    // Main entry point to benchmark concurrent queue
    // - Starts producer and consumer threads
    // - Waits for completion
    // - Returns tuple of pushed, popped counts and remaining queue size
    auto concurrent_queue_test(int producers, int consumers)
        -> std::tuple<int, int, std::size_t> {
        threads.reserve(producers + consumers);
        launch_producers(producers);
        launch_consumers(consumers);
        threads.clear();  // Automatically joins all jthreads
        return {pushed_con, popped_con, queue_con.size()};
    }

}  // namespace omp_bench
