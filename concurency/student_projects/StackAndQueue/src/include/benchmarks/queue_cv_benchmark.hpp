#pragma once

#include <string_view>

#include "two_stack_queue.hpp"
#include "benchmark_base.hpp"

class QueueCVBenchmark : public BenchmarkBase {
  private:
    two_stack_queue<int> m_queue;

  public:
    QueueCVBenchmark(std::string_view name,
                     int producers,
                     int consumers,
                     int total_items)
        : BenchmarkBase("TwoStackQueue (cv)",
                        producers,
                        consumers,
                        total_items) {}

  private:
    void producer_loop() override {
        for (int j = 0; j < m_items_per_producer; ++j) {
            m_queue.cv_enqueue(j);
            m_produced_count.fetch_add(1, std::memory_order_relaxed);
        }
    }

    void consumer_loop(int items_to_process) override {
        for (int j = 0; j < items_to_process; ++j) {
            m_queue.cv_dequeue_wait();
            m_consumed_count.fetch_add(1, std::memory_order_relaxed);
        }
    }
};