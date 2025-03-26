#include "benchmark.hpp"

#include <atomic>
#include <mutex>

namespace Benchmark {
    // Helper to measure time
    template <typename Func>
    auto measure_time(const std::string& name, Func&& func) -> void {
        auto start = std::chrono::high_resolution_clock::now();
        func();
        auto end = std::chrono::high_resolution_clock::now();
        auto duration =
            std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::println("{}: {} ms", name, duration.count());
    }

    // --- ConcurrentStack Benchmark ---
    auto benchmark_concurrent_stack(int N) -> void {
        ConcurrentStack<int> stack;
        std::atomic<int> push_count(0);
        std::atomic<int> pop_count(0);

        measure_time("ConcurrentStack (Push)", [&] {
            std::vector<std::jthread> threads;
            for (int i = 0; i < 2; ++i) {
                threads.emplace_back([&](std::stop_token stoken) {
                    while (push_count < N && !stoken.stop_requested()) {
                        stack.push(push_count);
                        push_count++;
                    }
                });
            }

            /*
            // Wait for completion
            while (push_count < N || pop_count < N) {
                if (push_count >= N && pop_count >= N) break;
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }*/
            while (push_count < N) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        });

        measure_time("ConcurrentStack (Pop)", [&] {
            std::vector<std::jthread> threads;
            for (int i = 0; i < 2; ++i) {
                threads.emplace_back([&](std::stop_token stoken) {
                    while (pop_count < N && !stoken.stop_requested()) {
                        if (!stack.empty()) {
                            stack.pop();
                            pop_count++;
                        }
                    }
                });
            }

            /*
            // Wait for completion
            while (push_count < N || pop_count < N) {
                if (push_count >= N && pop_count >= N) break;
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }*/
            while (pop_count < N) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        });

        std::println("Pushed: {} items", push_count.load());
        std::println("Popped: {} items", pop_count.load());
    }

    auto benchmark_concurrent_stack_producer_consumer(int N) -> void {
        ConcurrentStack<int> stack;
        std::atomic<int> push_count(0);
        std::atomic<int> pop_count(0);

        measure_time("ConcurrentStack (Producer-Consumer)", [&] {
            std::vector<std::jthread> prodThreads;
            for (int i = 0; i < 1; ++i) {
                prodThreads.emplace_back([&](std::stop_token stoken) {
                    while (push_count < N && !stoken.stop_requested()) {
                        stack.push(push_count);
                        push_count++;
                    }
                });
            }

            std::vector<std::jthread> consThreads;
            for (int i = 0; i < 1; ++i) {
                consThreads.emplace_back([&](std::stop_token stoken) {
                    while (pop_count < N && !stoken.stop_requested()) {
                        if (!stack.empty()) {
                            stack.pop();
                            pop_count++;
                        }
                    }
                });
            }

            // Wait for completion or timeout
            while (push_count < N || pop_count < N) {
                if (push_count >= N && pop_count >= N) break;
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }

            // Wait for completion
            while (push_count < N || pop_count < N) {
                if (push_count >= N && pop_count >= N) break;
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        });

        std::println("Pushed: {} items", push_count.load());
        std::println("Popped: {} items", pop_count.load());
    }

    // --- ConcurrentQueue Benchmark ---
    auto benchmark_concurrent_queue(int N) -> void {
        ConcurrentQueue<int> queue;
        std::atomic<int> push_count(0);
        std::atomic<int> pop_count(0);

        measure_time("ConcurrentQueue (Push)", [&] {
            std::vector<std::jthread> threads;
            for (int i = 0; i < 2; ++i) {
                threads.emplace_back([&](std::stop_token stoken) {
                    while (push_count < N && !stoken.stop_requested()) {
                        queue.push(push_count);
                        push_count++;
                    }
                });
            }

            // Wait for completion
            /*
            while (push_count < N || pop_count < N) {
                if (push_count >= N && pop_count >= N) break;
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
            */
            while (push_count < N) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        });

        measure_time("ConcurrentQueue (Pop)", [&] {
            std::vector<std::jthread> threads;
            for (int i = 0; i < 2; ++i) {
                threads.emplace_back([&](std::stop_token stoken) {
                    while (pop_count < N && !stoken.stop_requested()) {
                        if (queue.try_pop()) { pop_count++; }
                    }
                });
            }

            // Wait for completion
            /*
            while (push_count < N || pop_count < N) {
                if (push_count >= N && pop_count >= N) break;
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
            */
            while (pop_count < N) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        });

        std::println("Pushed: {} items", push_count.load());
        std::println("Popped: {} items", pop_count.load());
    }

    // --- ConcurrentQueue Producer-Consumer Benchmark ---
    auto benchmark_concurrent_queue_producer_consumer(int N) -> void {
        ConcurrentQueue<int> queue;
        std::atomic<int> push_count(0);
        std::atomic<int> pop_count(0);

        measure_time("ConcurrentQueue (Producer-Consumer)", [&] {
            std::vector<std::jthread> prodThreads;
            for (int i = 0; i < 1; ++i) {
                prodThreads.emplace_back([&](std::stop_token stoken) {
                    while (push_count < N && !stoken.stop_requested()) {
                        queue.push(push_count);
                        push_count++;
                    }
                });
            }

            std::vector<std::jthread> consThreads;
            for (int i = 0; i < 1; ++i) {
                consThreads.emplace_back([&](std::stop_token stoken) {
                    while (pop_count < N && !stoken.stop_requested()) {
                        if (queue.try_pop()) { pop_count++; }
                    }
                });
            }

            // Wait for completion
            while (push_count < N || pop_count < N) {
                if (push_count >= N && pop_count >= N) break;
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        });

        std::println("Pushed: {} items", push_count.load());
        std::println("Popped: {} items", pop_count.load());
    }

    // --- Stack Benchmark (Single-threaded) ---
    auto benchmark_stack(int N) -> void {
        Stack<int> stack;
        std::atomic<int> push_count(0);
        std::atomic<int> pop_count(0);

        measure_time("Stack (Push)", [&] {
            for (int push_count = 0; push_count < N; ++push_count)
                stack.push(push_count);
        });

        measure_time("Stack (Pop)", [&] {
            for (int pop_count = 0; pop_count < N; ++pop_count) stack.pop();
        });
    }

    // --- Queue Benchmark (Single-threaded) ---
    auto benchmark_queue(int N) -> void {
        Queue<int> queue;
        std::atomic<int> push_count(0);
        std::atomic<int> pop_count(0);

        measure_time("Queue (Push)", [&] {
            for (int push_count = 0; push_count < N; ++push_count)
                queue.push(push_count);
        });

        measure_time("Queue (Pop)", [&] {
            for (int pop_count = 0; pop_count < N; ++pop_count) queue.pop();
        });
    }

    // --- Run All Benchmarks ---
    auto run_benchmarks(int N) -> void {
        std::println("\n=== Benchmarking with N = {} ===\n", N);

        benchmark_concurrent_stack(N);
        benchmark_concurrent_stack_producer_consumer(N);
        benchmark_concurrent_queue(N);
        benchmark_concurrent_queue_producer_consumer(N);
        benchmark_stack(N);
        benchmark_queue(N);
    }
}  // namespace Benchmark