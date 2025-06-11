/**
 * @file benchmark_base.hpp
 * @brief Declares the abstract base class for all benchmark implementations.
 */

#pragma once

#include <atomic>
#include <chrono>
#include <string>
#include <thread>
#include <vector>

/**
 * @class benchmark_base
 * @brief Abstract interface for stack and queue benchmark implementations.
 */
class benchmark_base {
  protected:
    using Duration = std::chrono::milliseconds;
    static constexpr int one = 1;

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
    /**
     * @brief Constructs the benchmark with given parameters.
     * @param name Name of the benchmark for reporting.
     * @param producers Number of producer threads.
     * @param consumers Number of consumer threads.
     * @param total_items Total number of items to be processed.
     */
    benchmark_base(std::string_view name,
                   int producers,
                   int consumers,
                   int total_items);

    /**
     * @brief Virtual destructor.
     */
    virtual ~benchmark_base() = default;

    /**
     * @brief Prepares and runs the benchmark.
     */
    auto run() -> void;

    /**
     * @brief Reserves memory for thread containers.
     */
    auto prepare_threads() -> void;

    /**
     * @brief Prints the benchmark result to standard output.
     * @param duration Execution time in milliseconds.
     */
    auto print_result(Duration duration) -> void;

    /**
     * @brief Writes the benchmark result to a CSV file.
     * @param duration Execution time in milliseconds.
     * @param file_name Name of the output CSV file.
     */
    auto write_result_to_file(Duration duration, std::string_view file_name)
        -> void;

  protected:
    /**
     * @brief Logic for producer threads. Must be implemented by subclasses.
     */
    virtual auto producer_loop() -> void = 0;

    /**
     * @brief Logic for consumer threads. Must be implemented by subclasses.
     */
    virtual auto consumer_loop() -> void = 0;

  private:
    /**
     * @brief Starts all producer and consumer threads.
     */
    auto launch_threads() -> void;

    /**
     * @brief Waits for all threads to complete execution.
     */
    auto wait_for_completion() -> void;
};
