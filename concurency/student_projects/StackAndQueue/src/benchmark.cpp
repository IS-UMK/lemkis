#include "include/benchmark.hpp"

#include <atomic>
#include <chrono>
#include <mutex>
#include <print>
#include <thread>
#include <vector>

#include "include/con_queue.hpp"
#include "include/con_stack.hpp"
#include "include/list_stack.hpp"
#include "include/queue.hpp"
#include "include/stack.hpp"
#include "include/two_stack_queue.hpp"
#include "include/vector_stack.hpp"
#include "libs/concurrentqueue.h"
#include "libs/readerwriterqueue.h"

namespace benchmark {
    template <typename Func>
    static auto measure_time(const std::string& name, Func&& func) -> void {
        auto start = std::chrono::high_resolution_clock::now();
        func();
        auto end = std::chrono::high_resolution_clock::now();
        auto duration =
            std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::println("{}: {} ms", name, duration.count());
    }

    auto benchmark_concurrent_stack(int N) -> void {
        concurrent_stack<int> stack;
        std::atomic<int> push_count(0);
        std::atomic<int> pop_count(0);

        measure_time("concurrent_stack (Push)", [&] {
            std::vector<std::jthread> threads;
            threads.reserve(2);
            for (int i = 0; i < 2; ++i) {
                threads.emplace_back([&](const std::stop_token& stoken) {
                    while (push_count < N && !stoken.stop_requested()) {
                        stack.push(push_count);
                        push_count++;
                    }
                });
            }

            while (push_count < N) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        });

        measure_time("concurrent_stack (Pop)", [&] {
            std::vector<std::jthread> threads;
            threads.reserve(2);
            for (int i = 0; i < 2; ++i) {
                threads.emplace_back([&](const std::stop_token& stoken) {
                    while (pop_count < N && !stoken.stop_requested()) {
                        if (!stack.empty()) {
                            stack.pop();
                            pop_count++;
                        }
                    }
                });
            }

            while (pop_count < N) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        });

        std::println("Pushed: {} items", push_count.load());
        std::println("Popped: {} items", pop_count.load());
    }

    static auto benchmark_concurrent_stack_producer_consumer(int N) -> void {
        concurrent_stack<int> stack;
        std::atomic<int> push_count(0);
        std::atomic<int> pop_count(0);

        measure_time("concurrent_stack (Producer-Consumer)", [&] {
            std::vector<std::jthread> prod_threads;
            prod_threads.reserve(1);
            for (int i = 0; i < 1; ++i) {
                prod_threads.emplace_back([&](const std::stop_token& stoken) {
                    while (push_count < N && !stoken.stop_requested()) {
                        stack.push(push_count);
                        push_count++;
                    }
                });
            }

            std::vector<std::jthread> cons_threads;
            cons_threads.reserve(1);
            for (int i = 0; i < 1; ++i) {
                cons_threads.emplace_back([&](const std::stop_token& stoken) {
                    while (pop_count < N && !stoken.stop_requested()) {
                        if (!stack.empty()) {
                            stack.pop();
                            pop_count++;
                        }
                    }
                });
            }
            while (push_count < N || pop_count < N) {
                if (push_count >= N && pop_count >= N) { break; }
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
            while (push_count < N || pop_count < N) {
                if (push_count >= N && pop_count >= N) { break; }
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        });

        std::println("Pushed: {} items", push_count.load());
        std::println("Popped: {} items", pop_count.load());
    }

    auto benchmark_concurrent_queue(int N) -> void {
        concurrent_queue<int> queue;
        std::atomic<int> push_count(0);
        std::atomic<int> pop_count(0);

        measure_time("concurrent_queue (Push)", [&] {
            std::vector<std::jthread> threads;
            threads.reserve(2);
            for (int i = 0; i < 2; ++i) {
                threads.emplace_back([&](const std::stop_token& stoken) {
                    while (push_count < N && !stoken.stop_requested()) {
                        queue.push(push_count);
                        push_count++;
                    }
                });
            }
            while (push_count < N) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        });

        measure_time("concurrent_queue (Pop)", [&] {
            std::vector<std::jthread> threads;
            threads.reserve(2);
            for (int i = 0; i < 2; ++i) {
                threads.emplace_back([&](const std::stop_token& stoken) {
                    while (pop_count < N && !stoken.stop_requested()) {
                        if (queue.try_pop()) { pop_count++; }
                    }
                });
            }
            while (pop_count < N) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        });

        std::println("Pushed: {} items", push_count.load());
        std::println("Popped: {} items", pop_count.load());
    }

    static auto benchmark_concurrent_queue_producer_consumer(int N) -> void {
        concurrent_queue<int> queue;
        std::atomic<int> push_count(0);
        std::atomic<int> pop_count(0);

        measure_time("concurrent_queue (Producer-Consumer)", [&] {
            std::vector<std::jthread> prod_threads;
            prod_threads.reserve(1);
            for (int i = 0; i < 1; ++i) {
                prod_threads.emplace_back([&](const std::stop_token& stoken) {
                    while (push_count < N && !stoken.stop_requested()) {
                        queue.push(push_count);
                        push_count++;
                    }
                });
            }

            std::vector<std::jthread> cons_threads;
            cons_threads.reserve(1);
            for (int i = 0; i < 1; ++i) {
                cons_threads.emplace_back([&](const std::stop_token& stoken) {
                    while (pop_count < N && !stoken.stop_requested()) {
                        if (queue.try_pop()) { pop_count++; }
                    }
                });
            }
            while (push_count < N || pop_count < N) {
                if (push_count >= N && pop_count >= N) { break; }
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        });

        std::println("Pushed: {} items", push_count.load());
        std::println("Popped: {} items", pop_count.load());
    }

    auto benchmark_stack(int N) -> void {
        stack<int> stack;
        std::atomic<int> push_count(0);
        std::atomic<int> pop_count(0);

        measure_time("stack (Push)", [&] {
            while (push_count < N) {
                stack.push(push_count);
                push_count++;
            }
        });

        measure_time("stack (Pop)", [&] {
            while (pop_count < N) {
                stack.pop();
                pop_count++;
            }
        });
    }

    auto benchmark_queue(int N) -> void {
        queue<int> queue;
        std::atomic<int> push_count(0);
        std::atomic<int> pop_count(0);

        measure_time("queue (Push)", [&] {
            while (push_count < N) {
                queue.push(push_count);
                push_count++;
            }
        });

        measure_time("queue (Pop)", [&] {
            while (pop_count < N) {
                queue.pop();
                pop_count++;
            }
        });
    }

    auto run_benchmarks(int N) -> void {
        std::println("\n=== Benchmarking with N = {} ===\n", N);

        benchmark_concurrent_stack(N);
        benchmark_concurrent_stack_producer_consumer(N);
        benchmark_concurrent_queue(N);
        benchmark_concurrent_queue_producer_consumer(N);
        benchmark_stack(N);
        benchmark_queue(N);
    }

    template <typename StackType>
    static auto benchmark_stack_new(const std::string& stack_name,
                                    const std::string& method_name,
                                    int num_producers,
                                    int num_consumers,
                                    int items_per_producer) -> void {

        StackType stack;
        std::atomic<int> produced_count = 0;
        std::atomic<int> consumed_count = 0;
        std::atomic<bool> producers_done = false;

        // For timing
        auto start_time = std::chrono::high_resolution_clock::now();

        // Create producer threads
        std::vector<std::jthread> producers;
        for (int p = 0; p < num_producers; ++p) {
            if (method_name == "mutex") {
                producers.emplace_back(
                    [&stack, &produced_count, items_per_producer]() {
                        for (int i = 0; i < items_per_producer; ++i) {
                            stack.mutex_push(i);
                            produced_count.fetch_add(1);
                        }
                    });
            } else if (method_name == "cv") {
                producers.emplace_back(
                    [&stack, &produced_count, items_per_producer]() {
                        for (int i = 0; i < items_per_producer; ++i) {
                            stack.cv_push(i);
                            produced_count.fetch_add(1);
                        }
                    });
            }
        }

        // Create consumer threads
        std::vector<std::jthread> consumers;
        int total_items = num_producers * items_per_producer;
        int const items_per_consumer = total_items / num_consumers;

        for (int c = 0; c < num_consumers; ++c) {
            if (method_name == "mutex") {
                consumers.emplace_back([&stack,
                                        &consumed_count,
                                        &producers_done,
                                        items_per_consumer,
                                        total_items]() {
                    int items_processed = 0;
                    while (items_processed < items_per_consumer) {
                        auto item = stack.mutex_pop();
                        if (item) {
                            consumed_count.fetch_add(1);
                            items_processed++;
                        } else if (producers_done &&
                                   consumed_count.load() >= total_items) {
                            // All items have been produced and consumed
                            break;
                        } else {
                            // No items available yet, yield to other threads
                            std::this_thread::yield();
                        }
                    }
                });
            } else if (method_name == "cv") {
                consumers.emplace_back(
                    [&stack, &consumed_count, items_per_consumer]() {
                        for (int i = 0; i < items_per_consumer; ++i) {
                            stack.cv_pop_wait();
                            consumed_count.fetch_add(1);
                        }
                    });
            }
        }

        // Wait for all producer threads to finish
        for (auto& producer : producers) { producer.join(); }

        producers_done = true;

        // Wait for all consumer threads to finish
        for (auto& consumer : consumers) { consumer.join(); }

        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            end_time - start_time);

        std::print(
            "{} with {} method: {} producers, {} consumers, {} items total - "
            "completed in {} ms\n",
            stack_name,
            method_name,
            num_producers,
            num_consumers,
            total_items,
            duration.count());
    }

    // benchmark function for queue
    template <typename QueueType>
    static auto benchmark_queue_new(const std::string& queue_name,
                                    const std::string& method_name,
                                    int num_producers,
                                    int num_consumers,
                                    int items_per_producer) -> void {

        QueueType queue;
        std::atomic<int> produced_count = 0;
        std::atomic<int> consumed_count = 0;
        std::atomic<bool> producers_done = false;

        // For timing
        auto start_time = std::chrono::high_resolution_clock::now();

        // Create producer threads
        std::vector<std::jthread> producers;
        for (int p = 0; p < num_producers; ++p) {
            if (method_name == "mutex") {
                producers.emplace_back(
                    [&queue, &produced_count, items_per_producer]() {
                        for (int i = 0; i < items_per_producer; ++i) {
                            queue.mutex_enqueue(i);
                            produced_count.fetch_add(1);
                        }
                    });
            } else if (method_name == "cv") {
                producers.emplace_back(
                    [&queue, &produced_count, items_per_producer]() {
                        for (int i = 0; i < items_per_producer; ++i) {
                            queue.cv_enqueue(i);
                            produced_count.fetch_add(1);
                        }
                    });
            }
        }

        // Create consumer threads
        std::vector<std::jthread> consumers;
        int total_items = num_producers * items_per_producer;
        int const items_per_consumer = total_items / num_consumers;

        for (int c = 0; c < num_consumers; ++c) {
            if (method_name == "mutex") {
                consumers.emplace_back([&queue,
                                        &consumed_count,
                                        &producers_done,
                                        items_per_consumer,
                                        total_items]() {
                    int items_processed = 0;
                    while (items_processed < items_per_consumer) {
                        auto item = queue.mutex_dequeue();
                        if (item) {
                            consumed_count.fetch_add(1);
                            items_processed++;
                        } else if (producers_done &&
                                   consumed_count.load() >= total_items) {
                            // All items have been produced and consumed
                            break;
                        } else {
                            // No items available yet, yield to other
                            // threads
                            std::this_thread::yield();
                        }
                    }
                });
            } else if (method_name == "cv") {
                consumers.emplace_back(
                    [&queue, &consumed_count, items_per_consumer]() {
                        for (int i = 0; i < items_per_consumer; ++i) {
                            queue.cv_dequeue_wait();
                            consumed_count.fetch_add(1);
                        }
                    });
            }
        }

        // Wait for all producer threads to finish
        for (auto& producer : producers) { producer.join(); }

        producers_done = true;

        // Wait for all consumer threads to finish
        for (auto& consumer : consumers) { consumer.join(); }

        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            end_time - start_time);

        std::print(
            "{} with {} method: {} producers, {} consumers, {} items total "
            "- "
            "completed in {} ms\n",
            queue_name,
            method_name,
            num_producers,
            num_consumers,
            total_items,
            duration.count());
    }

    // benchmark function for queue
    template <typename QueueType>
    static auto benchmark_lock_free_queue_new(const std::string& queue_name,
                                              const std::string& mechanism_name,
                                              int num_producers,
                                              int num_consumers,
                                              int items_per_producer) -> void {

        QueueType queue;
        std::atomic<int> produced_count = 0;
        std::atomic<int> consumed_count = 0;
        std::atomic<bool> producers_done = false;

        // For timing
        auto start_time = std::chrono::high_resolution_clock::now();

        // Create producer threads
        std::vector<std::jthread> producers;
        producers.reserve(num_producers);
        for (int p = 0; p < num_producers; ++p) {
            producers.emplace_back(
                [&queue, &produced_count, items_per_producer]() {
                    for (int i = 0; i < items_per_producer; ++i) {
                        queue.enqueue(i);
                        produced_count.fetch_add(1);
                    }
                });
        }

        // Create consumer threads
        std::vector<std::jthread> consumers;
        int total_items = num_producers * items_per_producer;
        int const items_per_consumer = total_items / num_consumers;

        consumers.reserve(num_consumers);
        for (int c = 0; c < num_consumers; ++c) {
            consumers.emplace_back([&queue,
                                    &consumed_count,
                                    &producers_done,
                                    items_per_consumer,
                                    total_items]() {
                int items_processed = 0;
                while (items_processed < items_per_consumer) {
                    int item;
                    bool const succeeded = queue.try_dequeue(item);
                    if (succeeded) {
                        consumed_count.fetch_add(1);
                        items_processed++;
                    } else if (producers_done &&
                               consumed_count.load() >= total_items) {
                        // All items have been produced and consumed
                        break;
                    } else {
                        // No items available yet, yield to other
                        // threads
                        std::this_thread::yield();
                    }
                }
            });
        }

        // Wait for all producer threads to finish
        for (auto& producer : producers) { producer.join(); }

        producers_done = true;

        // Wait for all consumer threads to finish
        for (auto& consumer : consumers) { consumer.join(); }

        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            end_time - start_time);

        std::print(
            "{} with {} mechanism: {} producers, {} consumers, {} items total "
            "- "
            "completed in {} ms\n",
            queue_name,
            mechanism_name,
            num_producers,
            num_consumers,
            total_items,
            duration.count());
    }

    auto run_benchmarks_new(int N) -> void {
        // Configuration
        const int total_items = N;

        std::print("Running Stack Benchmarks:\n");
        std::print("========================\n\n");

        // Test various configurations for stacks
        for (int const producers : {1, 2, 4}) {
            for (int const consumers : {1, 2, 4}) {
                int const items_per_producer = total_items / producers;

                // Vector stack with mutex
                benchmark_stack_new<vector_stack<int>>("vector_stack",
                                                      "mutex",
                                                      producers,
                                                      consumers,
                                                      items_per_producer);

                // Vector stack with condition variables
                benchmark_stack_new<vector_stack<int>>("vector_stack",
                                                      "cv",
                                                      producers,
                                                      consumers,
                                                      items_per_producer);

                // List stack with mutex
                benchmark_stack_new<list_stack<int>>("list_stack",
                                                    "mutex",
                                                    producers,
                                                    consumers,
                                                    items_per_producer);

                // List stack with condition variables
                benchmark_stack_new<list_stack<int>>("list_stack",
                                                    "cv",
                                                    producers,
                                                    consumers,
                                                    items_per_producer);

                std::print("\n");
            }
        }

        std::print("\nRunning Queue Benchmarks:\n");
        std::print("========================\n\n");

        // Test various configurations for queue
        for (int const producers : {1, 2, 4}) {
            for (int const consumers : {1, 2, 4}) {
                int const items_per_producer = total_items / producers;

                // Two stack queue with mutex
                benchmark_queue_new<two_stack_queue<int>>("two_stack_queue",
                                                          "mutex",
                                                          producers,
                                                          consumers,
                                                          items_per_producer);

                // Two stack queue with condition variables
                benchmark_queue_new<two_stack_queue<int>>("two_stack_queue",
                                                          "cv",
                                                          producers,
                                                          consumers,
                                                          items_per_producer);

                std::print("\n");
            }
        }

        std::print("\nRunning Lock-Free Queue Benchmarks:\n");
        std::print("========================\n\n");

        // Test various configurations for queue
        for (int const producers : {1, 2, 4}) {
            for (int const consumers : {1, 2, 4}) {
                int const items_per_producer = total_items / producers;

                // Lock free queue with compare_exchange
                benchmark_lock_free_queue_new<moodycamel::ConcurrentQueue<int>>(
                    "LockFreeQueue",
                    "compare-exchange",
                    producers,
                    consumers,
                    items_per_producer);

                std::print("\n");
            }
        }

        // Lock free queue with atomic release
        benchmark_lock_free_queue_new<moodycamel::ReaderWriterQueue<int>>(
            "LockFreeQueue", "atomic", 1, 1, total_items);
    }
}  // namespace benchmark