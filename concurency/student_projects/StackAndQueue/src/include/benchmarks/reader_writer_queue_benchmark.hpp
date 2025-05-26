#pragma once

#include <string_view>

#include "readerwriterqueue.h"
#include "benchmark_base.hpp"

class ReaderWriterQueueBenchmark : public BenchmarkBase {
  private:
    moodycamel::ReaderWriterQueue<int> m_queue;

  public:
    ReaderWriterQueueBenchmark(std::string_view name, int total_items)
        : BenchmarkBase("moodyCamel:ReaderWriterQueue", 1, 1, total_items) {}

  private:
    void producer_loop() override {
        for (int j = 0; j < m_total_items; ++j) {
            m_queue.enqueue(j);
            m_produced_count.fetch_add(1, std::memory_order_relaxed);
        }
    }

    void consumer_loop(int items_to_process) override {
        int count = 0;
        while (count < items_to_process) {
            if (try_consume())
                ++count;
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
};