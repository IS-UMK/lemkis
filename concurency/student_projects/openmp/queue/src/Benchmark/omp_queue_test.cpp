#include "../include/omp_queue.hpp"

#include <omp.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <functional>
#include <iostream>
#include <numeric>
#include <print>
#include <string>
#include <thread>
#include <vector>

#include "../include/concurrent_queue.hpp"
#include "../include/queue_test.hpp"

constexpr int num_operations = 1'000'000;
constexpr int warmup_iterations = 2;
constexpr int measurement_iterations = 5;
constexpr int number_one = 1;
constexpr int number_two = 2;
constexpr int number_seventy_five = 75;
constexpr int one_hundred = 100;
constexpr double one_thousand = 1000.0;

namespace {

    // Shared barrier for thread synchronization
    std::atomic<int> barrier{0};
    std::vector<double> times;

    // Wrapper for concurrent_queue
    class concurrent_queue_wrapper {
      public:
        explicit concurrent_queue_wrapper(concurrent_queue<int>& q)
            : queue(q) {}
        concurrent_queue<int>& queue;

        auto get_queue() -> concurrent_queue<int>& { return queue; }

      public:
        void push(const int& item) { queue.push(item); }

        [[nodiscard]] auto static try_pop(concurrent_queue<int>& queue,
                                          int& result) -> bool {
            if (queue.try_peek(result)) { return queue.try_pop(); }
            return false;
        }

        [[nodiscard]] auto empty() const -> bool { return queue.empty(); }
        [[nodiscard]] auto size() const -> std::size_t { return queue.size(); }
    };

    struct benchmark_result {
        double mean_time_ms{};
        double median_time_ms{};
        double stddev_ms{};
        double cv_percent{};
        double throughput_ops_sec{};
        std::vector<double> raw_times;
    };

    struct benchmark_params {
        int num_producers{};
        int num_consumers{};
        int items_per_producer{};
        int total_items{};
    };

    auto calculate_workload(int num_producers) -> int {
        return num_operations / num_producers;
    }

    void synchronize_threads(int num_producers, int num_consumers) {
        barrier.fetch_add(number_one);
        const int total_threads = num_producers + num_consumers;
        while (barrier.load() < total_threads) { std::this_thread::yield(); }
    }

    struct producer_task_params {
        concurrent_queue_wrapper& queue;
        int& items_produced;
        int producer_id;
        int items_per_producer;
        int num_producers;
        int num_consumers;
    };

    void producer_task(const producer_task_params& params) {
        synchronize_threads(params.num_producers, params.num_consumers);
        for (int j = 0; j < params.items_per_producer; ++j) {
            params.queue.push((params.producer_id * params.items_per_producer) +
                              j);
#pragma omp atomic
            params.items_produced++;
        }
    }

    struct consumer_task_params {
        concurrent_queue_wrapper& queue;
        int& items_consumed;
        const int& items_produced;
        int total_items;
        int num_producers;
        int num_consumers;
    };

    template <typename QueueType>
    auto should_consumer_exit(const int& items_produced,
                              const int& items_consumed,
                              int total_items,
                              const QueueType& queue) -> bool {
        return (items_produced == total_items && queue.empty() &&
                items_consumed >= total_items);
    }

    auto has_reached_limit(const consumer_task_params& params) -> bool {
        return params.items_consumed >= params.total_items;
    }

    auto try_consume_item(const consumer_task_params& params, int& value)
        -> bool {
        return concurrent_queue_wrapper::try_pop(params.queue.get_queue(),
                                                 value);
    }

    auto should_exit_consumer(const consumer_task_params& params) -> bool {
        return should_consumer_exit(params.items_produced,
                                    params.items_consumed,
                                    params.total_items,
                                    params.queue);
    }

    void increment_consumed_count(const consumer_task_params& params) {
#pragma omp atomic
        params.items_consumed++;
    }

    void yield_thread() { std::this_thread::yield(); }

    // FIXED: Split consumer_task into smaller functions to reduce complexity
    auto process_consumer_item(const consumer_task_params& params, int& value)
        -> bool {
        if (try_consume_item(params, value)) {
            increment_consumed_count(params);
            return true;
        }
        return false;
    }

    auto should_continue_consuming(const consumer_task_params& params) -> bool {
        if (has_reached_limit(params)) { return false; }
        if (should_exit_consumer(params)) { return false; }
        return true;
    }

    void consumer_task(const consumer_task_params& params) {
        synchronize_threads(params.num_producers, params.num_consumers);
        int value;
        while (should_continue_consuming(params)) {
            if (!process_consumer_item(params, value)) { yield_thread(); }
        }
    }

    // Helper functions for thread creation
    template <typename QueueType>
    auto reserve_producer_vector(int num_producers)
        -> std::vector<std::thread> {
        std::vector<std::thread> producers;
        producers.reserve(static_cast<std::size_t>(num_producers));
        return producers;
    }

    template <typename QueueType>
    auto create_producer_params(QueueType& queue,
                                int& items_produced,
                                int producer_id,
                                const benchmark_params& params)
        -> producer_task_params {
        return producer_task_params{queue,
                                    items_produced,
                                    producer_id,
                                    params.items_per_producer,
                                    params.num_producers,
                                    params.num_consumers};
    }

    template <typename QueueType>
    auto create_producer_thread(QueueType& queue,
                                int& items_produced,
                                int producer_id,
                                const benchmark_params& params) -> std::thread {
        return std::thread([&, producer_id]() {
            auto task_params = create_producer_params(
                queue, items_produced, producer_id, params);
            producer_task(task_params);
        });
    }

    template <typename QueueType>
    auto create_producers(QueueType& queue,
                          int& items_produced,
                          const benchmark_params& params)
        -> std::vector<std::thread> {
        auto producers =
            reserve_producer_vector<QueueType>(params.num_producers);
        for (int i = 0; i < params.num_producers; ++i) {
            producers.emplace_back(
                create_producer_thread(queue, items_produced, i, params));
        }
        return producers;
    }

    template <typename QueueType>
    auto reserve_consumer_vector(int num_consumers)
        -> std::vector<std::thread> {
        std::vector<std::thread> consumers;
        consumers.reserve(static_cast<std::size_t>(num_consumers));
        return consumers;
    }

    template <typename QueueType>
    auto create_consumer_params(QueueType& queue,
                                int& items_consumed,
                                const int& items_produced,
                                const benchmark_params& params)
        -> consumer_task_params {
        return consumer_task_params{queue,
                                    items_consumed,
                                    items_produced,
                                    params.total_items,
                                    params.num_producers,
                                    params.num_consumers};
    }

    // Fixed: Reduced parameters by grouping them
    template <typename QueueType>
    auto create_consumer_thread(QueueType& queue,
                                int& items_consumed,
                                const int& items_produced,
                                const benchmark_params& params) -> std::thread {
        return std::thread([&]() {
            auto task_params = create_consumer_params(
                queue, items_consumed, items_produced, params);
            consumer_task(task_params);
        });
    }

    template <typename QueueType>
    auto create_consumers(QueueType& queue,
                          int& items_consumed,
                          const int& items_produced,
                          const benchmark_params& params)
        -> std::vector<std::thread> {
        auto consumers =
            reserve_consumer_vector<QueueType>(params.num_consumers);
        for (int i = 0; i < params.num_consumers; ++i) {
            consumers.emplace_back(create_consumer_thread(
                queue, items_consumed, items_produced, params));
        }
        return consumers;
    }

    void join_threads(std::vector<std::thread>& threads) {
        for (auto& t : threads) {
            if (t.joinable()) { t.join(); }
        }
    }

    template <typename QueueType>
    void run_benchmark_threads(QueueType& queue,
                               int& items_produced,
                               int& items_consumed,
                               const benchmark_params& params) {
        auto producers = create_producers(queue, items_produced, params);
        auto consumers =
            create_consumers(queue, items_consumed, items_produced, params);
        producers.insert(producers.end(),
                         std::make_move_iterator(consumers.begin()),
                         std::make_move_iterator(consumers.end()));
        join_threads(producers);
    }

    auto measure_execution_time(const std::function<void()>& func) -> double {
        auto start = std::chrono::high_resolution_clock::now();
        func();
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration<double, std::milli>(end - start).count();
    }

    void initialize_benchmark_state(int& items_produced, int& items_consumed) {
        items_produced = 0;
        items_consumed = 0;
        barrier.store(0);
    }

    auto create_benchmark_params(int num_producers, int num_consumers)
        -> benchmark_params {
        int const items_per_producer = calculate_workload(num_producers);
        int const total_items = items_per_producer * num_producers;
        return {.num_producers = num_producers,
                .num_consumers = num_consumers,
                .items_per_producer = items_per_producer,
                .total_items = total_items};
    }

    // Fixed: Split into smaller functions
    auto calculate_throughput(int total_items, double elapsed_ms) -> double {
        return (total_items * one_thousand) / elapsed_ms;
    }

    void print_result_line(const std::string& queue_name,
                           const benchmark_params& params,
                           double elapsed_ms,
                           double throughput) {
        std::print(
            "{:<25} | P: {:>2} | C: {:>2} | Time: {:.2f} ms | Throughput: "
            "{:.2f} ops/sec\n",
            queue_name,
            params.num_producers,
            params.num_consumers,
            elapsed_ms,
            throughput);
    }

    void print_benchmark_results(const std::string& queue_name,
                                 const benchmark_params& params,
                                 double elapsed_ms,
                                 bool warmup) {
        if (warmup) { return; }
        double const throughput =
            calculate_throughput(params.total_items, elapsed_ms);
        print_result_line(queue_name, params, elapsed_ms, throughput);
    }

    auto setup_stdthread_benchmark(int num_producers, int num_consumers)
        -> std::tuple<concurrent_queue_wrapper, int, int, benchmark_params> {
        concurrent_queue<int> real_queue;
        const concurrent_queue_wrapper queue_wrapper(real_queue);
        int items_produced{0};
        int items_consumed{0};
        initialize_benchmark_state(items_produced, items_consumed);
        auto params = create_benchmark_params(num_producers, num_consumers);

        return std::make_tuple(
            queue_wrapper, num_producers, num_consumers, params);
    }

    auto benchmark_stdthread(const std::string& queue_name,
                             int num_producers,
                             int num_consumers,
                             bool warmup = false) -> double {
        auto [queue, items_produced, items_consumed, params] =
            setup_stdthread_benchmark(num_producers, num_consumers);
        const double elapsed_ms = measure_execution_time([&]() {
            run_benchmark_threads(
                queue, items_produced, items_consumed, params);
        });
        print_benchmark_results(queue_name, params, elapsed_ms, warmup);
        return elapsed_ms;
    }

    // OpenMP-specific functions
    auto openmp_producer_task(auto& queue,
                              int thread_id,
                              int items_per_producer,
                              int& items_produced) -> void {
        for (int j = 0; j < items_per_producer; ++j) {
            queue.push((thread_id * items_per_producer) + j);
#pragma omp atomic
            items_produced++;
        }
    }

    auto should_openmp_consumer_exit(const int& items_produced,
                                     const int& items_consumed,
                                     int total_items,
                                     const auto& queue) -> bool {
        int const current_produced = items_produced;
        int const current_consumed = items_consumed;
        return (current_produced == total_items) && queue.empty() &&
               (current_consumed >= total_items);
    }

    auto try_consume_item(auto& queue, int& value, int& items_consumed)
        -> bool {
        const bool success =
            concurrent_queue_wrapper::try_pop(queue.get_queue(), value);
        if (success) {
#pragma omp atomic
            items_consumed++;
        }
        return success;
    }

    // Fixed: Split openmp_consumer_task
    auto should_consumer_continue(int items_consumed,
                                  int total_items,
                                  const int& items_produced,
                                  const auto& queue) -> bool {
        if (items_consumed >= total_items) { return false; }
        if (should_openmp_consumer_exit(
                items_produced, items_consumed, total_items, queue)) {
            return false;
        }
        return true;
    }

    auto openmp_consumer_task(auto& queue,
                              int total_items,
                              const int& items_produced,
                              int& items_consumed) -> void {
        int value{};
        while (true) {
            if (!should_consumer_continue(
                    items_consumed, total_items, items_produced, queue)) {
                break;
            }
            if (try_consume_item(queue, value, items_consumed)) { continue; }
#pragma omp taskyield
        }
    }

    struct thread_context {
        concurrent_queue_wrapper& queue;
        int& items_produced;
        int& items_consumed;
    };

    // Fixed: Split execute_thread_task
    void run_producer_task(int thread_id,
                           benchmark_params& params,
                           thread_context& ctx) {
        openmp_producer_task(ctx.queue,
                             thread_id,
                             params.items_per_producer,
                             ctx.items_produced);
    }

    void run_consumer_task(benchmark_params& params, thread_context& ctx) {
        openmp_consumer_task(ctx.queue,
                             params.total_items,
                             ctx.items_produced,
                             ctx.items_consumed);
    }

    void execute_thread_task(int thread_id,
                             benchmark_params& params,
                             thread_context& ctx) {
        if (thread_id < params.num_producers) {
            run_producer_task(thread_id, params, ctx);
        } else {
            run_consumer_task(params, ctx);
        }
    }

    auto setup_openmp_benchmark(int p, int c)
        -> std::tuple<concurrent_queue_wrapper, int, int, benchmark_params> {
        concurrent_queue<int> q;
        const concurrent_queue_wrapper qwrap(q);
        int produced = 0;
        int consumed = 0;
        initialize_benchmark_state(produced, consumed);
        auto params = create_benchmark_params(p, c);
        return std::make_tuple(qwrap, produced, consumed, params);
    }


    void run_openmp_parallel_section(benchmark_params& params,
                                     thread_context& ctx) {
#pragma omp parallel num_threads(params.num_producers + params.num_consumers)
        { execute_thread_task(omp_get_thread_num(), params, ctx); }
    }

    auto execute_openmp_benchmark(int num_producers, int num_consumers)
        -> std::tuple<thread_context, benchmark_params> {
        auto [queue, items_produced, items_consumed, params] =
            setup_openmp_benchmark(num_producers, num_consumers);
        const thread_context ctx{.queue = queue,
                                 .items_produced = items_produced,
                                 .items_consumed = items_consumed};
        return {ctx, params};
    }

    auto benchmark_openmp(const std::string& queue_name,
                          int num_producers,
                          int num_consumers,
                          bool warmup = false) -> double {
        auto [ctx, params] =
            execute_openmp_benchmark(num_producers, num_consumers);
        const double elapsed_ms = measure_execution_time(
            [&]() { run_openmp_parallel_section(params, ctx); });
        print_benchmark_results(queue_name, params, elapsed_ms, warmup);
        return elapsed_ms;
    }


    // Warmup and measurement helpers
    void perform_warmup_runs(const std::string& queue_name,
                             int num_producers,
                             int num_consumers,
                             auto benchmark_function) {
        for (int i = 0; i < warmup_iterations; ++i) {
            benchmark_function(
                queue_name + " (warmup)", num_producers, num_consumers, true);
        }
    }

    // Fixed: Split collect_timing_data
    auto create_run_name(const std::string& queue_name, int run_number)
        -> std::string {
        return queue_name + " (run " + std::to_string(run_number) + ")";
    }

    struct benchmark_parameters {
        std::string queue_name;
        int iteration;
        int num_producers;
        int num_consumers;
    };

    auto run_single_benchmark(const benchmark_parameters& params,
                              auto benchmark_function) -> double {
        const std::string run_name =
            create_run_name(params.queue_name, params.iteration + number_one);
        return benchmark_function(
            run_name, params.num_producers, params.num_consumers, false);
    }

    auto collect_timing_data(const std::string& queue_name,
                             int num_producers,
                             int num_consumers,
                             auto benchmark_function) -> std::vector<double> {
        times.reserve(measurement_iterations);
        for (int i = 0; i < measurement_iterations; ++i) {
            benchmark_parameters const params{.queue_name = queue_name,
                                              .iteration = i,
                                              .num_producers = num_producers,
                                              .num_consumers = num_consumers};
            times.push_back(run_single_benchmark(params, benchmark_function));
        }
        return times;
    }


    // Fixed: Split calculate_mean_stddev
    auto calculate_mean(const std::vector<double>& times) -> double {
        const double sum = std::accumulate(times.begin(), times.end(), 0.0);
        return sum / static_cast<double>(times.size());
    }

    auto calculate_stddev(const std::vector<double>& times, double mean)
        -> double {
        std::vector<double> diff(times.size());
        std::ranges::transform(
            times, diff.begin(), [mean](double x) { return x - mean; });
        const double sq_sum =
            std::inner_product(diff.begin(), diff.end(), diff.begin(), 0.0);
        return std::sqrt(sq_sum / static_cast<double>(times.size()));
    }

    auto calculate_mean_stddev(const std::vector<double>& times)
        -> std::pair<double, double> {
        const double mean = calculate_mean(times);
        const double stddev = calculate_stddev(times, mean);
        return {mean, stddev};
    }

    auto calculate_median(std::vector<double> times) -> double {
        std::ranges::sort(times);
        return times[times.size() / number_two];
    }

    auto create_benchmark_result(const std::vector<double>& times)
        -> benchmark_result {
        auto [mean, stddev] = calculate_mean_stddev(times);
        const double median = calculate_median(times);
        return {.mean_time_ms = mean,
                .median_time_ms = median,
                .stddev_ms = stddev,
                .cv_percent = (stddev / mean) * one_hundred,
                .throughput_ops_sec =
                    (static_cast<double>(num_operations) * one_thousand) / mean,
                .raw_times = times};
    }

    void print_statistical_summary(const std::string& queue_name,
                                   const benchmark_result& result) {
        std::print("\n--- {} Statistics ---\n", queue_name);
        std::print("Mean: {:.2f} ms\n", result.mean_time_ms);
        std::print("Median: {:.2f} ms\n", result.median_time_ms);
        std::print("StdDev: {:.2f} ms\n", result.stddev_ms);
        std::print("Coefficient of Variation: {:.2f}%\n", result.cv_percent);
        std::print("Throughput: {:.2f} ops/sec\n", result.throughput_ops_sec);
        std::print("---------------------------\n\n");
    }

    template <typename QueueType, typename BenchmarkFunc>
    auto run_benchmark_suite(const std::string& queue_name,
                             int num_producers,
                             int num_consumers,
                             BenchmarkFunc benchmark_function)
        -> benchmark_result {
        perform_warmup_runs(
            queue_name, num_producers, num_consumers, benchmark_function);
        auto times = collect_timing_data(
            queue_name, num_producers, num_consumers, benchmark_function);
        auto result = create_benchmark_result(times);
        print_statistical_summary(queue_name, result);
        return result;
    }

    void print_benchmark_header(int num_producers, int num_consumers) {
        std::print("\n=====================================================\n");
        std::print("Running benchmark with {} producers and {} consumers\n",
                   num_producers,
                   num_consumers);
        std::print("=====================================================\n");
    }

    // Benchmark runners for specific configurations
    auto run_omp_stdthread_benchmark(int num_producers, int num_consumers)
        -> benchmark_result {
        return run_benchmark_suite<concurrent_queue_wrapper>(
            "omp_queue + std::thread",
            num_producers,
            num_consumers,
            benchmark_stdthread);
    }

    auto run_omp_openmp_benchmark(int num_producers, int num_consumers)
        -> benchmark_result {
        return run_benchmark_suite<concurrent_queue_wrapper>(
            "omp_queue + OpenMP",
            num_producers,
            num_consumers,
            benchmark_openmp);
    }

    auto run_concurrent_stdthread_benchmark(int num_producers,
                                            int num_consumers)
        -> benchmark_result {
        return run_benchmark_suite<concurrent_queue_wrapper>(
            "ConcurrentQueue + std::thread",
            num_producers,
            num_consumers,
            benchmark_stdthread);
    }

    auto run_concurrent_openmp_benchmark(int num_producers, int num_consumers)
        -> benchmark_result {
        return run_benchmark_suite<concurrent_queue_wrapper>(
            "ConcurrentQueue + OpenMP",
            num_producers,
            num_consumers,
            benchmark_openmp);
    }

    // Aggregate all results
    struct all_benchmark_results {
        benchmark_result omp_stdthread;
        benchmark_result omp_openmp;
        benchmark_result concurrent_stdthread;
        benchmark_result concurrent_openmp;
    };

    auto run_all_benchmarks(int num_producers, int num_consumers)
        -> all_benchmark_results {
        return all_benchmark_results{
            .omp_stdthread =
                run_omp_stdthread_benchmark(num_producers, num_consumers),
            .omp_openmp =
                run_omp_openmp_benchmark(num_producers, num_consumers),
            .concurrent_stdthread = run_concurrent_stdthread_benchmark(
                num_producers, num_consumers),
            .concurrent_openmp =
                run_concurrent_openmp_benchmark(num_producers, num_consumers)};
    }

    void print_comparison_header(int num_producers, int num_consumers) {
        std::print("Configuration: {} producers, {} consumers\n",
                   num_producers,
                   num_consumers);
        std::print("{:<30}{:<15}{:<15}{:<15}\n",
                   "Implementation",
                   "Mean Time (ms)",
                   "Throughput",
                   "CV (%)");
        std::print("{}\n", std::string(number_seventy_five, '-'));
    }

    void print_result_row(const std::string& name,
                          const benchmark_result& result) {
        std::print("{:<30}{:<15.2f}{:<15.2f}{:<15.2f}\n",
                   name,
                   result.mean_time_ms,
                   result.throughput_ops_sec,
                   result.cv_percent);
    }

    void print_comparison_table(const all_benchmark_results& results) {
        print_result_row("omp_queue + std::thread", results.omp_stdthread);
        print_result_row("omp_queue + OpenMP", results.omp_openmp);
        print_result_row("ConcurrentQueue + std::thread",
                         results.concurrent_stdthread);
        print_result_row("ConcurrentQueue + OpenMP", results.concurrent_openmp);
        std::print("\n---------------------------\n\n");
    }

    // Fixed: Split print_relative_performance
    void print_relative_header() {
        std::print(
            "Relative Performance (lower is better, omp_queue + std::thread = "
            "1.0):\n");
    }

    void print_relative_times(const all_benchmark_results& results,
                              double base_time) {
        std::print("omp_queue + OpenMP: {:.2f}x\n",
                   results.omp_openmp.mean_time_ms / base_time);
        std::print("ConcurrentQueue + std::thread: {:.2f}x\n",
                   results.concurrent_stdthread.mean_time_ms / base_time);
        std::print("ConcurrentQueue + OpenMP: {:.2f}x\n",
                   results.concurrent_openmp.mean_time_ms / base_time);
    }

    void print_relative_performance(const all_benchmark_results& results) {
        const double base_time = results.omp_stdthread.mean_time_ms;
        print_relative_header();
        print_relative_times(results, base_time);
        std::print("\n=====================================================\n");
    }


}  // namespace
// FIXED: Changed from static to anonymous namespace function
auto run_full_comparison(int num_producers, int num_consumers) -> void {
    print_benchmark_header(num_producers, num_consumers);
    auto results = run_all_benchmarks(num_producers, num_consumers);
    print_comparison_header(num_producers, num_consumers);
    print_comparison_table(results);
    print_relative_performance(results);
}