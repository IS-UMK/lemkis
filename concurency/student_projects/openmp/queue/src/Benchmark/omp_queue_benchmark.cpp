// --------------------------------------------------------------
// Includes and Constants
// --------------------------------------------------------------

#include <omp.h>

#include <benchmark_utils.hpp>
#include <omp_queue.hpp>


// --------------------------------------------------------------
// Internal State and OpenMP Logic (anonymous namespace)
// --------------------------------------------------------------

namespace {
    omp_queue<int> queue;  // OpenMP-compatible thread-safe queue
    int pushed = 0;        // Total number of items pushed
    int popped = 0;        // Total number of items popped
    int const id_offset =
        10000;                  // Offset used to differentiate producers' data
    int const k_increment = 1;  // Increment value for atomic counters
    // Producer function executed by threads with ID < number of producers
    // Each thread pushes 'per' items with unique base offset
    auto omp_producer(int tid, int per) -> void {
        for (int i = 0; i < per; ++i) {
            queue.push(i + (tid * id_offset));
#pragma omp atomic
            pushed += k_increment;
        }
    }

    // Atomically increments the 'popped' counter
    auto atomic_pop_increment() {
#pragma omp atomic
        popped += k_increment;
    }

    // Tries to pop an item from the queue if the total hasn't been reached
    auto try_pop(int total) -> bool {
        int current;
#pragma omp atomic read
        current = popped;
        if (current >= total) { return false; }
        if (queue.pop()) {
            atomic_pop_increment();
            return true;
        }
        return false;
    }

    // Performs one consumer action; yields if nothing was popped
    auto consumer_action(int total) -> bool {
        if (!try_pop(total)) {
#pragma omp taskyield
            int cur;
#pragma omp atomic read
            cur = popped;
            return cur >= total;
        }
        return false;
    }

    // Consumer thread loop: keeps consuming until all items are popped
    auto omp_consumer_loop(int total) -> void {
        while (true) {
            if (consumer_action(total)) { break; }
        }
    }

    // Launches a parallel region with producers and consumers
    auto run_omp_threads(int prod,
                         [[maybe_unused]] int cons,
                         int per,
                         int total) -> void {
#pragma omp parallel num_threads(prod + cons)
        {
            int const tid = omp_get_thread_num();
            if (tid < prod) {
                omp_producer(tid, per);
            } else {
                omp_consumer_loop(total);
            }
        }
    }
}  // namespace

// --------------------------------------------------------------
// Benchmark Interface (omp_bench namespace)
// --------------------------------------------------------------

namespace omp_bench {

    // Entry point for benchmarking the OpenMP queue
    // - Initializes per-thread work
    // - Runs OpenMP threads with producers and consumers
    // - Returns pushed, popped counts and remaining queue size
    auto omp_queue_test(int producers, int consumers)
        -> std::tuple<int, int, std::size_t> {
        int const item_limit = 1000000;  // Total number of items to be pushed
        int const per = item_limit / producers;
        int const total = per * producers;
        run_omp_threads(producers, consumers, per, total);
        return {pushed, popped, queue.size()};
    }

}  // namespace omp_bench
