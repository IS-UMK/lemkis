/**
 * @file stack_cv_benchmark.hpp
 * @brief Benchmark for a condition-variable synchronized stack implementation.
 */

#pragma once

#include <benchmark_base.hpp>
#include <string_view>

/**
 * @class stack_cv_benchmark
 * @brief Benchmark using a stack synchronized with condition variables.
 *
 * This benchmark evaluates performance of condition-variable-based stacks
 * with multiple producer and consumer threads.
 *
 * Intended for use with vector_stack and list_stack passed as template
 * parameter.
 *
 * @tparam StackType Stack container implementing cv_push and cv_pop_wait.
 */
template <typename StackType>
class stack_cv_benchmark : public benchmark_base {
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
    stack_cv_benchmark(std::string_view name,
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
            m_stack.cv_push(j);
            m_produced_count.fetch_add(one, std::memory_order_relaxed);
        }
    }

    /**
     * @brief Function executed by each consumer thread.
     */
    auto consumer_loop() -> void override {
        for (int j = 0; j < m_items_per_consumer; ++j) {
            m_stack.cv_pop_wait();
            m_consumed_count.fetch_add(one, std::memory_order_relaxed);
        }
    }
};
