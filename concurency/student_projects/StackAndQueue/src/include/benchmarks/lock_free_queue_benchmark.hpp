#pragma once

#include <string_view>

#include "concurrentqueue.h"
#include "benchmark_base.hpp"

class LockFreeQueueBenchmark : public BenchmarkBase {
  private:
    moodycamel::ConcurrentQueue<int> m_queue;

  public:
    LockFreeQueueBenchmark(std::string_view name,
                           int producers,
                           int consumers,
                           int total_items)
        : BenchmarkBase("moodycamel:ConcurrentQueue",
                        producers,
                        consumers,
                        total_items) {}

  private:
    void producer_loop() override {
        for (int j = 0; j < m_items_per_producer; ++j) {
            m_queue.enqueue(j);
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
        int value;
        bool success = m_queue.try_dequeue(value);
        if (!success) return false;
        m_consumed_count.fetch_add(1, std::memory_order_relaxed);
        return true;
    }

    bool should_break() const {
        return m_producers_done.load(std::memory_order_acquire) &&
               m_consumed_count.load(std::memory_order_relaxed) >=
                   m_total_items;
    }
};