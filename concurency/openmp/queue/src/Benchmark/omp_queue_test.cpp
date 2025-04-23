#include "../include/omp_queue.hpp"

#include <chrono>
#include <format>
#include <iostream>
#include <print>

#include "../include/concurrent_queue.hpp"
// #include <jthread>
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
        int local_count = 0;
        int value = 0;
        while (!queue.empty()) {
            if (queue.pop(value)) {
                ++popped_count;
                ++local_count;
            }
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

    const auto start = std::chrono::high_resolution_clock::now();

#pragma omp parallel num_threads(producers + consumers)
    {
        const int thread_id = omp_get_thread_num();
        if (thread_id < producers) {
            for (int i = 0; i < item_limit / producers; ++i) {
                queue.push(i + thread_id * id_offset);
#pragma omp atomic
                ++pushed_count;
            }
        } else {
            int local_count = 0;
            int value = 0;
            while (!queue.empty()) {
                if (queue.pop(value)) {
#pragma omp atomic
                    ++popped_count;
                    ++local_count;
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


// Runs jthread-based producer-consumer logic using concurrent_queue
void run_jthread_concurrentqueue_test(int producers, int consumers) {
    concurrent_queue<int> queue;
    int pushed_count = 0;
    int popped_count = 0;
    std::mutex count_mutex;

    const auto start = std::chrono::high_resolution_clock::now();

    std::vector<std::jthread> threads;

    for (int i = 0; i < producers; ++i) {
        threads.emplace_back(
            [&queue, &pushed_count, i, producers, &count_mutex]() {
                for (int j = 0; j < item_limit / producers; ++j) {
                    queue.push(j + i * id_offset);
                    std::scoped_lock lock(count_mutex);
                    ++pushed_count;
                }
            });
    }

    for (int i = 0; i < consumers; ++i) {
        threads.emplace_back(
            [&queue, &popped_count, i, consumers, &count_mutex]() {
                int local_count = 0;
                int value = 0;
                while (local_count < item_limit / consumers) {
                    if (queue.try_peek(value)) {
                        queue.pop();
                        std::scoped_lock lock(count_mutex);
                        ++popped_count;
                        ++local_count;
                    }
                }
            });
    }

    const auto end = std::chrono::high_resolution_clock::now();

    print_performance(std::format("[ConcurrentQueue] With std::jthread - "
                                    "Producers: {}, Consumers: {}",
                                    producers,
                                    consumers),
                        pushed_count,
                        popped_count,
                        queue.size(),
                        std::chrono::duration<double>(end - start).count());
}