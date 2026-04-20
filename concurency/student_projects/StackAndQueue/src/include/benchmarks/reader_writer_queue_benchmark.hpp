/**
 * @file reader_writer_queue_benchmark.hpp
 * @brief Benchmark for a lock-free single-producer single-consumer queue.
 */

#pragma once

#include <readerwriterqueue.h>

#include <benchmark_base.hpp>
#include <string_view>
#include <thread>

/**
 * @class reader_writer_queue_benchmark
 * @brief Benchmark using moodycamel::ReaderWriterQueue (SPSC lock-free queue).
 *
 * This benchmark evaluates performance of the single-producer, single-consumer
 * queue under one producer and one consumer thread.
 */
class reader_writer_queue_benchmark : public benchmark_base {
  private:
    moodycamel::ReaderWriterQueue<int> m_queue;

  public:
    /**
     * @brief Constructs the benchmark for a single-producer single-consumer
     * setup.
     * @param name Benchmark label for output.
     * @param total_items Total number of items to process.
     */
    reader_writer_queue_benchmark(std::string_view name, int total_items)
        : benchmark_base(name, one, one, total_items) {}

  private:
    /**
     * @brief Function executed by the single producer thread.
     */
    auto producer_loop() -> void override {
        for (int j = 0; j < m_items_per_producer; ++j) {
            m_queue.enqueue(j);
            m_produced_count.fetch_add(one, std::memory_order_relaxed);
        }
    }

    /**
     * @brief Function executed by the single consumer thread.
     */
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

    /**
     * @brief Attempts to dequeue one item from the queue.
     * @return true if an item was consumed, false otherwise.
     */
    auto try_consume() -> bool {
        int value;
        if (!m_queue.try_dequeue(value)) { return false; }
        m_consumed_count.fetch_add(one, std::memory_order_relaxed);
        return true;
    }
};
