#pragma once

#include <benchmark_base.hpp>
#include <string_view>

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
    auto producer_loop() -> void override {
        for (int j = 0; j < m_items_per_producer; ++j) {
            m_stack.mutex_push(j);
            m_produced_count.fetch_add(one, std::memory_order_relaxed);
        }
    }

    auto consumer_loop() -> void override {
        int count = 0;
        while (!should_break(count)) {
            if (try_consume()) {
                ++count;
                continue;
            }
            std::this_thread::yield();
        }
    }

    auto try_consume() -> bool {
        if (!m_stack.mutex_pop()) { return false; }
        m_consumed_count.fetch_add(one, std::memory_order_relaxed);
        return true;
    }

    auto should_break(int count) -> bool {
        return count >= m_items_per_consumer ||
               (m_producers_done.load(std::memory_order_acquire) &&
                m_consumed_count.load(std::memory_order_relaxed) >=
                    m_total_items);
    }
};
