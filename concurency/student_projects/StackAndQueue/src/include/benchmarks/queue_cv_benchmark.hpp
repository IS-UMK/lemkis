/**
 * @file queue_cv_benchmark.hpp
 * @brief Benchmark for a condition-variable synchronized two-stack queue.
 */

#pragma once

#include <benchmark_base.hpp>
#include <string_view>
#include <two_stack_queue.hpp>

/**
 * @class queue_cv_benchmark
 * @brief Benchmark using two_stack_queue with condition-variable
 * synchronization.
 *
 * This benchmark evaluates condition-variable-based queue performance
 * with multiple producer and consumer threads.
 */
class queue_cv_benchmark : public benchmark_base {
  private:
    two_stack_queue<int> m_queue;

  public:
    /**
     * @brief Constructs the benchmark with the specified configuration.
     * @param name Benchmark label for output.
     * @param producers Number of producer threads.
     * @param consumers Number of consumer threads.
     * @param total_items Total number of items to process.
     */
    queue_cv_benchmark(std::string_view name,
                       int producers,
                       int consumers,
                       int total_items)
        : benchmark_base(name, producers, consumers, total_items) {}

  private:
    /**
     * @brief Function executed by each producer thread.
     */
    auto producer_loop() -> void override {
        for (int j = 0; j < m_items_per_producer; ++j) {
            m_queue.cv_enqueue(j);
            m_produced_count.fetch_add(one, std::memory_order_relaxed);
        }
    }

    /**
     * @brief Function executed by each consumer thread.
     */
    auto consumer_loop() -> void override {
        for (int j = 0; j < m_items_per_consumer; ++j) {
            m_queue.cv_dequeue_wait();
            m_consumed_count.fetch_add(one, std::memory_order_relaxed);
        }
    }
};
