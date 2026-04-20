/**
 * @file stack_mutex_benchmark.hpp
 * @brief Benchmark for a mutex-protected stack implementation.
 */

#pragma once

#include <benchmark_base.hpp>
#include <string_view>

/**
 * @class stack_mutex_benchmark
 * @brief Benchmark using a stack protected by a mutex.
 *
 * This benchmark evaluates stack performance under mutex-based synchronization
 * with multiple producer and consumer threads.
 *
 * Intended for use with vector_stack and list_stack passed as template
 * parameter.
 *
 * @tparam StackType Stack container implementing mutex_push and mutex_pop.
 */
template <typename StackType>
class stack_mutex_benchmark : public benchmark_base {
  private:
    StackType m_stack;

  public:
    /**
     * @brief Constructs the benchmark with the specified configuration.
     * @param name Benchmark label for output.
     * @param producers Number of producer threads.
     * @param consumers Number of consumer threads.
     * @param total_items Total number of items to process.
     */
    stack_mutex_benchmark(std::string_view name,
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
            m_stack.mutex_push(j);
            m_produced_count.fetch_add(one, std::memory_order_relaxed);
        }
    }

    /**
     * @brief Function executed by each consumer thread.
     */
    auto consumer_loop() -> void override {
        int count = 0;
        while (!should_break(count)) {
            if (try_consume()) {
                ++count;
                continue;
            }
            std::this_thread::yield();
        }
    }

    /**
     * @brief Attempts to pop one item from the stack.
     * @return true if an item was consumed, false otherwise.
     */
    auto try_consume() -> bool {
        if (!m_stack.mutex_pop()) { return false; }
        m_consumed_count.fetch_add(one, std::memory_order_relaxed);
        return true;
    }

    /**
     * @brief Checks if the consumer should stop consuming.
     * @param count Number of items consumed by this thread.
     * @return true if the loop should exit.
     */
    auto should_break(int count) -> bool {
        return count >= m_items_per_consumer ||
               (m_producers_done.load(std::memory_order_acquire) &&
                m_consumed_count.load(std::memory_order_relaxed) >=
                    m_total_items);
    }
};
