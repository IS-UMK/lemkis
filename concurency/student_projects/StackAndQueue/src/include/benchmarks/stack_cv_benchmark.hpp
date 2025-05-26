#pragma once

#include <string_view>
#include "benchmark_base.hpp"

template <typename StackType>
class StackCVBenchmark : public BenchmarkBase {
  private:
    StackType m_stack;

  public:
    StackCVBenchmark(std::string_view name, int producers, int consumers, int total_items)
        : BenchmarkBase(name, producers, consumers, total_items) {}

  private:
    void producer_loop() override {
        for (int j = 0; j < m_items_per_producer; ++j) {
            m_stack.cv_push(j);
            m_produced_count.fetch_add(1, std::memory_order_relaxed);
        }
    }

    void consumer_loop() override {
        for (int j = 0; j < m_items_per_consumer; ++j) {
            m_stack.cv_pop_wait();
            m_consumed_count.fetch_add(1, std::memory_order_relaxed);
        }
    }
};