#include <benchmark_base.hpp>
#include <format>
#include <fstream>
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

auto benchmark_base::prepare_threads() -> void {
    m_producers.reserve(m_num_producers);
    m_consumers.reserve(m_num_consumers);
}

auto benchmark_base::run() -> void {
    launch_threads();
    wait_for_completion();
}

auto benchmark_base::launch_threads() -> void {
    std::generate_n(std::back_inserter(m_producers), m_num_producers, [this] {
        return std::jthread([this] { producer_loop(); });
    });

    std::generate_n(std::back_inserter(m_consumers), m_num_consumers, [this] {
        return std::jthread([this] { consumer_loop(); });
    });
}

auto benchmark_base::wait_for_completion() -> void {
    for (auto& p : m_producers) { p.join(); }
    m_producers_done.store(true, std::memory_order_release);
    for (auto& c : m_consumers) { c.join(); }
}

auto benchmark_base::print_result(Duration duration) -> void {
    std::print("{}: {} producers, {} consumers, {} items total - {} ms\n",
               m_name,
               m_num_producers,
               m_num_consumers,
               m_total_items,
               duration.count());
}

auto benchmark_base::write_result_to_file(Duration duration,
                                          std::string_view file_name) -> void {
    if (std::ofstream out(file_name.data(), std::ios::app); out) {
        std::string formatted = std::format("{},{},{},{},{}\n",
                                            m_name,
                                            m_num_producers,
                                            m_num_consumers,
                                            m_total_items,
                                            duration.count());
        out.write(formatted.data(), formatted.size());
    }
}