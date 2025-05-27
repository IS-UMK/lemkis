#pragma once

#include <string_view>
#include <thread>

#include "benchmark_base.hpp"
#include "readerwriterqueue.h"

class reader_writer_queue_benchmark : public benchmark_base {
  private:
    moodycamel::ReaderWriterQueue<int> m_queue;

  public:
    reader_writer_queue_benchmark(std::string_view name, int total_items)
        : benchmark_base(name, one, one, total_items) {}

  private:
    auto producer_loop() -> void override {
        for (int j = 0; j < m_items_per_producer; ++j) {
            m_queue.enqueue(j);
            m_produced_count.fetch_add(one, std::memory_order_relaxed);
        }
    }

    auto consumer_loop() -> void override {
        int count = 0;
        while (count < m_items_per_consumer) {
            if (try_consume()) {
                ++count;
                continue;
            }
            std::this_thread::yield();
        }
    }

    auto try_consume() -> bool {
        int value;
        if (!m_queue.try_dequeue(value)) { return false; }
        m_consumed_count.fetch_add(one, std::memory_order_relaxed);
        return true;
    }
};