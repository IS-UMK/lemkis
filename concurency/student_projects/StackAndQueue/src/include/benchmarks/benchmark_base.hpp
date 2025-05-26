#pragma once

#include <atomic>
#include <chrono>
#include <jthread>
#include <string_view>
#include <vector>

class BenchmarkBase {
  protected:
    using Clock = std::chrono::high_resolution_clock;
    using Duration = std::chrono::milliseconds;

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
    BenchmarkBase(std::string_view name, int producers, int consumers, int total_items);
    virtual ~BenchmarkBase() = default;
    void run();

  protected:
    virtual void producer_loop() = 0;
    virtual void consumer_loop(int items_to_process) = 0;

  private:
    void prepare_threads();
    void launch_threads();
    void wait_for_completion();
    void print_result(Duration duration) const;
};
