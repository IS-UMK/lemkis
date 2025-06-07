#include <benchmark_utils.hpp>
#include <concurrent_queue.hpp>
#include <thread>
#include <vector>
constexpr int item_limit = 1000000;
constexpr int id_offset = 10000;
constexpr int k_increment = 1;

namespace {
    std::atomic<bool> done{false};
    concurrent_queue<int> queue_con;
    std::atomic<int> pushed_con;
    std::atomic<int> popped_con;
    std::vector<std::jthread> threads;


    void push_items(int base, int count) {
        for (int j = 0; j < count; ++j) {
            queue_con.push(j + base);
            pushed_con.fetch_add(k_increment, std::memory_order_relaxed);
        }
        if (pushed_con.load() >= item_limit) { done.store(true); }
    }

    void consume_items() {
        while (true) {
            if (done.load() && queue_con.empty()) { break; }
            if (queue_con.try_pop()) {
                popped_con.fetch_add(k_increment, std::memory_order_relaxed);
            } else {
                std::this_thread::yield();
            }
        }
    }

    void launch_producers(int count) {
        int const per = item_limit / count;
        for (int i = 0; i < count; ++i) {
            threads.emplace_back(push_items, i * id_offset, per);
        }
    }

    void launch_consumers(int count) {
        for (int i = 0; i < count; ++i) { threads.emplace_back(consume_items); }
    }
}  // namespace

namespace omp_bench {
    auto concurrent_queue_test(int producers, int consumers)
        -> std::tuple<int, int, std::size_t> {
        threads.reserve(producers + consumers);
        launch_producers(producers);
        launch_consumers(consumers);
        threads.clear();  // joins all threads
        return {pushed_con, popped_con, queue_con.size()};
    }
}  // namespace omp_bench