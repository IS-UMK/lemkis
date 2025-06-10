#pragma once

#include <benchmark_base.hpp>
#include <string_view>

template <typename StackType>
class stack_cv_benchmark : public benchmark_base {
  private:
    StackType m_stack;

  public:
    stack_cv_benchmark(std::string_view name,
                       int producers,
                       int consumers,
                       int total_items)
        : benchmark_base(name, producers, consumers, total_items) {}

  private:
    auto producer_loop() -> void override {
        for (int j = 0; j < m_items_per_producer; ++j) {
            m_stack.cv_push(j);
            m_produced_count.fetch_add(one, std::memory_order_relaxed);
        }
    }

    auto consumer_loop() -> void override {
        for (int j = 0; j < m_items_per_consumer; ++j) {
            m_stack.cv_pop_wait();
            m_consumed_count.fetch_add(one, std::memory_order_relaxed);
        }
    }
};