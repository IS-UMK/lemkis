#include "../include/omp_queue.hpp"

#include <chrono>
#include <format>
#include <iostream>
#include <print>

#include "../include/concurrent_queue.hpp"
#include <mutex>
#include <thread>
#include <utility>
#include <vector>


constexpr int item_limit = 1000000;
constexpr int id_offset = 10000;


// Measures and prints performance metrics of a queue test
void print_performance(const std::string& label,
                       int pushed,
                       int popped,
                       std::size_t remaining,
                       double time_seconds) {
    std::print("\n{}\n", label);
    std::print("Pushed: {}, Popped: {}, Remaining: {}, Time: {:.4f} s\n",
               pushed,
               popped,
               remaining,
               time_seconds);
}


// Runs producer and consumer logic for OMPQueue without OpenMP
void run_sequential_ompqueue_test(int producers, int consumers) {
    OMPQueue<int> queue;
    int pushed_count = 0;
    int popped_count = 0;

    const auto start = std::chrono::high_resolution_clock::now();

    for (int producer_id = 0; producer_id < producers; ++producer_id) {
        for (int j = 0; j < item_limit / producers; ++j) {
            queue.push(j + producer_id * id_offset);
            ++pushed_count;
        }
    }

    for (int consumer_id = 0; consumer_id < consumers; ++consumer_id) {
        int value = 0;
        while (popped_count < item_limit) {
            if (queue.pop(value)) { ++popped_count; }
        }

    }

    const auto end = std::chrono::high_resolution_clock::now();

    print_performance(
        std::format("[OMPQueue] No OpenMP - Producers: {}, Consumers: {}",
                    producers,
                    consumers),
        pushed_count,
        popped_count,
        queue.size(),
        std::chrono::duration<double>(end - start).count());
}


// Runs OpenMP-based producer-consumer logic using OMPQueue
void run_parallel_ompqueue_test(int producers, int consumers) {
    OMPQueue<int> queue;
    int pushed_count = 0;
    int popped_count = 0;

    const int items_per_producer = item_limit / producers;
    const int total_to_push = items_per_producer * producers;

    const auto start = std::chrono::high_resolution_clock::now();

#pragma omp parallel num_threads(producers + consumers)
    {
        const int thread_id = omp_get_thread_num();

        if (thread_id < producers) {
            for (int i = 0; i < items_per_producer; ++i) {
                queue.push(i + thread_id * id_offset);
#pragma omp atomic
                ++pushed_count;
            }
        } else {
            int value = 0;
            while (true) {
                int current;
#pragma omp atomic read
                current = popped_count;
                if (current >= total_to_push) { break; }

                if (queue.pop(value)) {
#pragma omp atomic
                    ++popped_count;
                } else {
#pragma omp taskyield
                }
            }
        }
    }

    const auto end = std::chrono::high_resolution_clock::now();

    print_performance(
        std::format("[OMPQueue] With OpenMP - Producers: {}, Consumers: {}",
                    producers,
                    consumers),
        pushed_count,
        popped_count,
        queue.size(),
        std::chrono::duration<double>(end - start).count());
}



// Runs jthread-based producer-consumer logic using concurrent_queue and
// stop_token
void run_jthread_concurrentqueue_test(int producers, int consumers) {
    concurrent_queue<int> queue;
    std::atomic<int> pushed_count{0};
    std::atomic<int> popped_count{0};
    std::vector<std::jthread> threads;

    const int items_per_producer = item_limit / producers;
    const auto start = std::chrono::high_resolution_clock::now();

    // Create producers
    for (int i = 0; i < producers; ++i) {
        threads.emplace_back([&, i](const std::stop_token& stoken) {
            for (int j = 0; j < items_per_producer && !stoken.stop_requested();
                 ++j) {
                queue.push(j + i * id_offset);
                pushed_count.fetch_add(1, std::memory_order_relaxed);
                std::this_thread::yield();
            }
        });
    }

    // Create consumers
    for (int i = 0; i < consumers; ++i) {
        threads.emplace_back([&](const std::stop_token& stoken) {
            while (!stoken.stop_requested() || !queue.empty()) {
                if (queue.try_pop()) {
                    popped_count.fetch_add(1, std::memory_order_relaxed);
                }
                std::this_thread::yield();
            }
        });
    }

    // Let threads work for reasonable time (scaled with problem size)
    std::this_thread::sleep_for(
        std::chrono::milliseconds(100 + (item_limit / 10000)));

    // Request all threads to stop
    for (auto& thread : threads) { thread.request_stop(); }

    // Ensure all threads finish
    threads.clear();

    const auto end = std::chrono::high_resolution_clock::now();

    print_performance(std::format("[ConcurrentQueue] With std::jthread - "
                                  "Producers: {}, Consumers: {}",
                                  producers,
                                  consumers),
                      pushed_count.load(),
                      popped_count.load(),
                      queue.size(),
                      std::chrono::duration<double>(end - start).count());
}