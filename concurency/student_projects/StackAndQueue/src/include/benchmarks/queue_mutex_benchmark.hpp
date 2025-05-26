#pragma once

#include <string_view>

#include "two_stack_queue.hpp"
#include "benchmark_base.hpp"

class QueueMutexBenchmark : public BenchmarkBase {
  private:
    two_stack_queue<int> m_queue;

  public:
    QueueMutexBenchmark(std::string_view name,
                        int producers,
                        int consumers,
                        int total_items)
        : BenchmarkBase("TwoStackQueue (mutex)",
                        producers,
                        consumers,
                        total_items) {}

  private:
    void producer_loop() override {
        for (int j = 0; j < m_items_per_producer; ++j) {
            m_queue.mutex_enqueue(j);
            m_produced_count.fetch_add(1, std::memory_order_relaxed);
        }
    }

    void consumer_loop(int items_to_process) override {
        int count = 0;
        while (count < items_to_process) {
            if (try_consume())
                ++count;
            else if (should_break())
                break;
            else
                std::this_thread::yield();
        }
    }

    bool try_consume() {
        auto item = m_queue.mutex_dequeue();
        if (!item) return false;
        m_consumed_count.fetch_add(1, std::memory_order_relaxed);
        return true;
    }

    bool should_break() const {
        return m_producers_done.load(std::memory_order_acquire) &&
               m_consumed_count.load(std::memory_order_relaxed) >=
                   m_total_items;
    }
};
