#pragma once

#include <string_view>

#include "benchmark_base.hpp"
#include "../structures/two_stack_queue.hpp"

class queue_cv_benchmark : public benchmark_base {
  private:
    two_stack_queue<int> m_queue;

  public:
    queue_cv_benchmark(std::string_view name,
                       int producers,
                       int consumers,
                       int total_items)
        : benchmark_base(name, producers, consumers, total_items) {}

  private:
    void producer_loop() override {
        for (int j = 0; j < m_items_per_producer; ++j) {
            m_queue.cv_enqueue(j);
            m_produced_count.fetch_add(one, std::memory_order_relaxed);
        }
    }

    void consumer_loop() override {
        for (int j = 0; j < m_items_per_consumer; ++j) {
            m_queue.cv_dequeue_wait();
            m_consumed_count.fetch_add(one, std::memory_order_relaxed);
        }
    }
};