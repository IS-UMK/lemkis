#pragma once

#include <fstream>
#include <list_stack.hpp>
#include <lock_free_queue_benchmark.hpp>
#include <memory>
#include <print>
#include <queue_cv_benchmark.hpp>
#include <queue_mutex_benchmark.hpp>
#include <ranges>
#include <reader_writer_queue_benchmark.hpp>
#include <stack_cv_benchmark.hpp>
#include <stack_mutex_benchmark.hpp>
#include <stream_utils.hpp>
#include <timer.hpp>
#include <vector>
#include <vector_stack.hpp>

namespace benchmark_script {
    using vector_stack_t = vector_stack<int>;
    using list_stack_t = list_stack<int>;

    static constexpr int single_producer = 1;
    static constexpr int single_consumer = 1;

    inline auto add_stack_benchmarks(
        std::vector<std::unique_ptr<benchmark_base>>& benchmark,
        int producers_count,
        int consumers_count,
        int elements_count) -> void {
        benchmark.emplace_back(
            std::make_unique<stack_mutex_benchmark<vector_stack_t>>(
                "vector_stack (mutex)",
                producers_count,
                consumers_count,
                elements_count));
        benchmark.emplace_back(
            std::make_unique<stack_cv_benchmark<vector_stack_t>>(
                "vector_stack (cv)",
                producers_count,
                consumers_count,
                elements_count));
        benchmark.emplace_back(
            std::make_unique<stack_mutex_benchmark<list_stack_t>>(
                "list_stack (mutex)",
                producers_count,
                consumers_count,
                elements_count));
        benchmark.emplace_back(
            std::make_unique<stack_cv_benchmark<list_stack_t>>(
                "list_stack (cv)",
                producers_count,
                consumers_count,
                elements_count));
    }

    inline auto add_queue_benchmarks(
        std::vector<std::unique_ptr<benchmark_base>>& benchmark,
        int producers_count,
        int consumers_count,
        int elements_count) -> void {
        benchmark.emplace_back(
            std::make_unique<queue_mutex_benchmark>("two_stack_queue (mutex)",
                                                    producers_count,
                                                    consumers_count,
                                                    elements_count));
        benchmark.emplace_back(
            std::make_unique<queue_cv_benchmark>("two_stack_queue (cv)",
                                                 producers_count,
                                                 consumers_count,
                                                 elements_count));
    }

    inline auto add_lockfree_benchmarks(
        std::vector<std::unique_ptr<benchmark_base>>& benchmark,
        int producers_count,
        int consumers_count,
        int elements_count) -> void {
        benchmark.emplace_back(std::make_unique<lock_free_queue_benchmark>(
            "moodycamel::ConcurrentQueue",
            producers_count,
            consumers_count,
            elements_count));
        if (producers_count == single_producer &&
            consumers_count == single_consumer) {
            benchmark.emplace_back(
                std::make_unique<reader_writer_queue_benchmark>(
                    "moodycamel::ReaderWriterQueue", elements_count));
        }
    }

    inline auto create_all_benchmarks(int producers_count,
                                      int consumers_count,
                                      int elements_count)
        -> std::vector<std::unique_ptr<benchmark_base>> {
        std::vector<std::unique_ptr<benchmark_base>> benchmark;
        add_stack_benchmarks(
            benchmark, producers_count, consumers_count, elements_count);
        add_queue_benchmarks(
            benchmark, producers_count, consumers_count, elements_count);
        add_lockfree_benchmarks(
            benchmark, producers_count, consumers_count, elements_count);
        return benchmark;
    }

    inline auto run_and_report(
        std::vector<std::unique_ptr<benchmark_base>> benchmark,
        std::string_view file_name) -> void {
        for (auto& bench : benchmark) {
            timer t;
            bench->prepare_threads();
            t.start();
            bench->run();
            const auto duration = t.elapsed();
            bench->print_result(duration);
            bench->write_result_to_file(duration, file_name);
        }
    }

    inline auto run_for_config(int producers_count,
                               int consumers_count,
                               int elements_count,
                               std::string_view file_name) -> void {
        std::print("{} producer(s), {} consumer(s):\n",
                   producers_count,
                   consumers_count);
        auto benchmarks = create_all_benchmarks(
            producers_count, consumers_count, elements_count);
        run_and_report(std::move(benchmarks), file_name);
        std::print("\n");
    }

    inline auto run_all_benchmarks(const std::vector<int>& producer_counts,
                                   const std::vector<int>& consumer_counts,
                                   int total_items,
                                   std::string_view file_name) -> void {
        if (std::ofstream out(file_name.data()); out) {
            std::string header(
                "benchmark,producers,consumers,items,duration_ms\n");
            out.write(header.data(), to_streamsize(header.size()));
        }
        std::print("Running all benchmarks:\n========================\n\n");
        for (const int producer_count : producer_counts) {
            for (const int consumer_count : consumer_counts) {
                run_for_config(
                    producer_count, consumer_count, total_items, file_name);
            }
        }
    }

    inline auto run_all_configurations(std::string_view file_name) -> void {
        constexpr int elements_per_benchmark = 30000;
        const std::vector<int> producer_counts{1, 2, 4};
        const std::vector<int> consumer_counts{1, 2, 4};
        run_all_benchmarks(producer_counts,
                           consumer_counts,
                           elements_per_benchmark,
                           file_name);
    }
}  // namespace benchmark_script
