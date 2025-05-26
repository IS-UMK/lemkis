#pragma once

#include <atomic>
#include <chrono>
#include <jthread>
#include <string_view>
#include <vector>

class benchmark_base {
  protected:
    using Clock = std::chrono::high_resolution_clock;
    using Duration = std::chrono::milliseconds;
    constexpr int m_one = 1;

    int m_num_producers;
    int m_num_consumers;
    int m_items_per_producer;
    int m_items_per_consumer;
    int m_total_items;

    std::string_view m_name;

    std::atomic<int> m_produced_count = 0;
    std::atomic<int> m_consumed_count = 0;
    std::atomic<bool> m_producers_done = false;

    std::vector<std::jthread> m_producers;
    std::vector<std::jthread> m_consumers;

  public:
    benchmark_base(std::string_view name,
                   int producers,
                   int consumers,
                   int total_items);
    virtual ~benchmark_base() = default;
    auto run() -> void;

  protected:
    virtual auto producer_loop() -> void;
    virtual auto consumer_loop() -> void;

  private:
    auto prepare_threads() -> void;
    auto launch_threads() -> void;
    auto wait_for_completion() -> void;
    auto print_result(Duration duration) -> void;
};
