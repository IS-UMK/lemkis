#include "benchmark_base.hpp"

#include <print>

benchmark_base::benchmark_base(std::string_view name,
                             int producers,
                             int consumers,
                             int total_items)
    : m_num_producers{producers},
      m_num_consumers{consumers},
      m_total_items{total_items},
      m_name{name} {
    m_items_per_producer = total_items / producers;
    m_items_per_consumer = total_items / consumers;
}

auto benchmark_base::run() -> void {
    prepare_threads();
    const auto start = Clock::now();
    launch_threads();
    wait_for_completion();
    const auto end = Clock::now();
    print_result(std::chrono::duration_cast<Duration>(end - start));
}

auto benchmark_base::prepare_threads() -> void {
    m_producers.reserve(m_num_producers);
    m_consumers.reserve(m_num_consumers);
}

auto benchmark_base::launch_threads() const -> void {
    for (int i = 0; i < m_num_producers; ++i) {
        m_producers.emplace_back([this] { producer_loop(); });
    }
    for (int i = 0; i < m_num_consumers; ++i) {
        m_consumers.emplace_back(
            [this] { consumer_loop(m_items_per_consumer); });
    }
}

auto benchmark_base::wait_for_completion() -> void {
    for (auto& p : m_producers) p.join();
    m_producers_done.store(true, std::memory_order_release);
    for (auto& c : m_consumers) c.join();
}

auto benchmark_base::print_result(Duration duration) -> void {
    std::print("{}: {} producers, {} consumers, {} items total - {} ms\n",
               m_name,
               m_num_producers,
               m_num_consumers,
               m_total_items,
               duration.count());
}