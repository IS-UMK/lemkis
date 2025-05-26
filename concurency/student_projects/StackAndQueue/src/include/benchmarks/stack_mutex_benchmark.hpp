#pragma once

#include <string_view>

#include "benchmark_base.hpp"

template <typename StackType>
class stack_mutex_benchmark : public benchmark_base {
  private:
    StackType m_stack;

  public:
    stack_mutex_benchmark(std::string_view name,
                          int producers,
                          int consumers,
                          int total_items)
        : benchmark_base(name, producers, consumers, total_items) {}

  private:
    void producer_loop() override {
        for (int j = 0; j < m_items_per_producer; ++j) {
            m_stack.mutex_push(j);
            m_produced_count.fetch_add(1, std::memory_order_relaxed);
        }
    }

    void consumer_loop(int items_to_process) override {
        int count = 0;
        while (count < items_to_process) {
            if (try_consume()) {
                ++count;
            } else if (should_break()) {
                break;
            } else {
                std::this_thread::yield();
            }
        }
    }

    bool try_consume() {
        auto item = m_stack.mutex_pop();
        if (!item) { return false; }
        m_consumed_count.fetch_add(1, std::memory_order_relaxed);
        return true;
    }

    bool should_break() const {
        return m_producers_done.load(std::memory_order_acquire) &&
               m_consumed_count.load(std::memory_order_relaxed) >=
                   m_total_items;
    }
};
