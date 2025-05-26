#pragma once

#include <string_view>

#include "benchmark_base.hpp"
#include "concurrentqueue.h"

class lock_free_queue_benchmark : public benchmark_base {
  private:
    moodycamel::ConcurrentQueue<int> m_queue;

  public:
    lock_free_queue_benchmark(std::string_view name,
                              int producers,
                              int consumers,
                              int total_items)
        : benchmark_base(name, producers, consumers, total_items) {}

  private:
    auto producer_loop() override -> void {
        for (int j = 0; j < m_items_per_producer; ++j) {
            m_queue.enqueue(j);
            m_produced_count.fetch_add(m_one, std::memory_order_relaxed);
        }
    }

    auto consumer_loop() override -> void {
        int count = 0;
        while (count < m_items_per_consumer) {
            if (try_consume()) {
                ++count;
            } else if (should_break()) {
                break;
            } else {
                std::this_thread::yield();
            }
        }
    }

    auto try_consume() -> bool {
        int value;
        bool success = m_queue.try_dequeue(value);
        if (!success) { return false; }
        m_consumed_count.fetch_add(m_one, std::memory_order_relaxed);
        return true;
    }

    auto should_break() const -> bool {
        return m_producers_done.load(std::memory_order_acquire) &&
               m_consumed_count.load(std::memory_order_relaxed) >=
                   m_total_items;
    }
};