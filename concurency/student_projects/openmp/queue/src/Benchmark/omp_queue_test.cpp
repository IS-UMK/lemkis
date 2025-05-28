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

    // Wrapper for concurrent_queue
    class concurrent_queue_wrapper {
    private:
        concurrent_queue<int> queue;

    public:
        void push(const int& item) { queue.push(item); }

        [[nodiscard]] static auto try_pop(concurrent_queue<int>& queue, int& result) -> bool {
            if (queue.try_peek(result)) {
                return queue.try_pop();
            }
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
        while (barrier.load() < total_threads) {
            std::this_thread::yield();
        }
    }

    struct producer_task_params {
        concurrent_queue_wrapper& queue;
        std::atomic<int>& items_produced;
        int producer_id;
        int items_per_producer;
        int num_producers;
        int num_consumers;
    };

    void producer_task(const producer_task_params& params) {
        synchronize_threads(params.num_producers, params.num_consumers);
        for (int j = 0; j < params.items_per_producer; ++j) {
            params.queue.push((params.producer_id * params.items_per_producer) + j);
            params.items_produced.fetch_add(number_one);
        }
    }

    struct consumer_task_params {
        concurrent_queue_wrapper& queue;
        std::atomic<int>& items_consumed;
        const std::atomic<int>& items_produced;
        int total_items;
        int num_producers;
        int num_consumers;
    };

    void consumer_task(const consumer_task_params& params) {
        synchronize_threads(params.num_producers, params.num_consumers);
        int const value{};
        while (true) {
            int current_consumed = params.items_consumed.load();
            if (current_consumed >= params.total_items) { break; }
            if (concurrent_queue_wrapper::try_pop(params.queue, value)) {params.items_consumed.fetch_add(number_one)};
             else {
                if (should_consumer_exit(params.items_produced, params.items_consumed, params.total_items, params.queue)){break};
                std::this_thread::yield();}}
    }

    // Helper functions for thread creation
    template <typename QueueType>
    auto create_producers(QueueType& queue, std::atomic<int>& items_produced, const benchmark_params& params) -> std::vector<std::thread> {
        std::vector<std::thread> producers;
        producers.reserve(static_cast<std::size_t>(params.num_producers));
        for (int i = 0; i < params.num_producers; ++i) {
            producers.emplace_back([&, i]() {
                ProducerTaskParams task_params{queue, items_produced, i, params.items_per_producer, params.num_producers, params.num_consumers};
                producer_task(task_params);
            });
        }
        return producers;
    }

    template <typename QueueType>
    auto create_consumers(QueueType& queue, std::atomic<int>& items_consumed,
                          const std::atomic<int>& items_produced,
                          const benchmark_params& params) -> std::vector<std::thread> {
        std::vector<std::thread> consumers;
        consumers.reserve(static_cast<std::size_t>(params.num_consumers));
        for (int i = 0; i < params.num_consumers; ++i) {
            consumers.emplace_back([&, i]() {
                ConsumerTaskParams task_params{queue, items_consumed, items_produced, params.total_items, params.num_producers, params.num_consumers};
                consumer_task(task_params);
            });
        }
        return consumers;
    }

    void join_threads(std::vector<std::thread>& threads) {
        for (auto& t : threads) {
            if (t.joinable()) {
                t.join();
            }
        }
    }

    template <typename QueueType>
    void run_benchmark_threads(QueueType& queue,
                               std::atomic<int>& items_produced,
                               std::atomic<int>& items_consumed,
                               const benchmark_params& params) {
        auto producers = create_producers(queue, items_produced, params);
        auto consumers = create_consumers(queue, items_consumed, items_produced, params);
        // Merge threads
        producers.insert(producers.end(), std::make_move_iterator(consumers.begin()), std::make_move_iterator(consumers.end()));
        join_threads(producers);
    }

    auto measure_execution_time(const std::function<void()>& func) -> double {
        auto start = std::chrono::high_resolution_clock::now();
        func();
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration<double, std::milli>(end - start).count();
    }

    void initialize_benchmark_state(std::atomic<int>& items_produced, std::atomic<int>& items_consumed) {
        items_produced.store(0);
        items_consumed.store(0);
        barrier.store(0);
    }

    auto create_benchmark_params(int num_producers, int num_consumers) -> benchmark_params {
        int const items_per_producer = calculate_workload(num_producers);
        int const total_items = items_per_producer * num_producers;
        return {.num_producers = num_producers,
                .num_consumers = num_consumers,
                .items_per_producer = items_per_producer,
                .total_items = total_items};
    }

    void print_benchmark_results(const std::string& queue_name, const benchmark_params& params, double elapsed_ms, bool warmup) {
        if (warmup) { return; }
        double throughput = (params.total_items * one_thousand) / elapsed_ms;
        std::print("{:<25} | P: {:>2} | C: {:>2} | Time: {:.2f} ms | Throughput: {:.2f} ops/sec\n",
                   queue_name, params.num_producers, params.num_consumers, elapsed_ms, throughput);
    }

} // end anonymous namespace
namespace{
// Benchmark with std::thread
auto benchmark_stdthread(const std::string& queue_name,int num_producers,int num_consumers,bool warmup = false) -> double {
    concurrent_queue_wrapper queue;
    std::atomic<int> items_produced{0};
    std::atomic<int> items_consumed{0};
    initialize_benchmark_state(items_produced, items_consumed);
    auto params = create_benchmark_params(num_producers, num_consumers);
    double elapsed_ms = measure_execution_time([&]() {run_benchmark_threads(queue, items_produced, items_consumed, params);});
    print_benchmark_results(queue_name, params, elapsed_ms, warmup);
    return elapsed_ms;
}
}
// OpenMP-specific functions

namespace {
    auto openmp_producer_task(auto& queue, int thread_id, int items_per_producer, std::atomic<int>& items_produced) -> void {
        for (int j = 0; j < items_per_producer; ++j) {
            queue.push((thread_id * items_per_producer) + j);
#pragma omp atomic
            items_produced++;
        }
    }

    auto should_openmp_consumer_exit(const std::atomic<int>& items_produced,
                                     const std::atomic<int>& items_consumed,
                                     int total_items,
                                     const auto& queue) -> bool {
        int const current_produced{};
        int const current_consumed{};
#pragma omp atomic read
        current_produced = items_produced;
#pragma omp atomic read
        current_consumed = items_consumed;
        return (current_produced == total_items) && queue.empty() && (current_consumed >= total_items);
    }

    auto openmp_consumer_task(auto& queue, int total_items, const std::atomic<int>& items_produced, std::atomic<int>& items_consumed) -> void {
        int value{};
        while (true) {
            if (items_consumed.load() >= total_items || should_openmp_consumer_exit(
                items_produced, items_consumed, total_items, queue)) {break;}
            if (try_consume_item(queue, value, items_consumed)) {
                continue;
            }
#pragma omp taskyield // Yield if no items were consumed
        }
    }
    auto try_consume_item(auto& queue, int& value, std::atomic<int>& items_consumed) -> bool {
        if (auto success = concurrent_queue_wrapper::try_pop(queue, value); success) {
            #pragma omp atomic
            items_consumed++;
        }
        return success;
    }


    struct thread_context {
        concurrent_queue_wrapper& queue;
        std::atomic<int>& items_produced;
        std::atomic<int>& items_consumed;
    };

    void execute_thread_task(int thread_id,
                             BenchmarkParams& params,
                             thread_context& ctx) {
        if (thread_id < params.num_producers) {
            openmp_producer_task(ctx.queue, thread_id, params.items_per_producer, ctx.items_produced);
        } else {
            openmp_consumer_task(ctx.queue, params.total_items, ctx.items_produced, ctx.items_consumed);
        }
    }


// Funkcja benchmarkująca OpenMP
auto benchmark_openmp(const std::string& queue_name, int num_producers, int num_consumers, bool warmup = false) -> double {
    concurrent_queue_wrapper const queue;
    std::atomic<int> items_produced{0};
    std::atomic<int> items_consumed{0};
    auto params = create_benchmark_params(num_producers, num_consumers);
    double elapsed_ms = measure_execution_time([&]() {
        #pragma omp parallel num_threads = num_producers + num_consumers
        {execute_thread_task(omp_get_thread_num(), queue, params, items_produced, items_consumed);}});
    print_benchmark_results(queue_name, params, elapsed_ms, warmup);
    return elapsed_ms;}
}

// Warmup and measurement helpers
namespace {

    void perform_warmup_runs(const std::string& queue_name,
                             int num_producers,
                             int num_consumers,
                             auto benchmark_function) {
        for (int i = 0; i < warmup_iterations; ++i) {
            benchmark_function(queue_name + " (warmup)", num_producers, num_consumers, true);
        }
    }

    auto collect_timing_data(const std::string& queue_name,
                             int num_producers,
                             int num_consumers,
                             auto benchmark_function) -> std::vector<double> {
        std::vector<double> times;
        times.reserve(measurement_iterations);
        for (int i = 0; i < measurement_iterations; ++i) {
            std::string run_name = queue_name + " (run " + std::to_string(i + number_one) + ")";
            double time = benchmark_function(run_name, num_producers, num_consumers, false);
            times.push_back(time);
        }
        return times;
    }

    auto calculate_mean_stddev(const std::vector<double>& times) -> std::pair<double, double> {
        double sum = std::accumulate(times.begin(), times.end(), 0.0);
        double mean = sum / static_cast<double>(times.size());

        std::vector<double> diff(times.size());
        std::transform(times.begin(), times.end(), diff.begin(), [mean](double x) { return x - mean; });
        double sq_sum = std::inner_product(diff.begin(), diff.end(), diff.begin(), 0.0);
        double stddev = std::sqrt(sq_sum / static_cast<double>(times.size()));
        return {mean, stddev};
    }

    auto calculate_median(std::vector<double> times) -> double {
        std::ranges::sort(times);
        return times[times.size() / number_two];
    }

    auto create_benchmark_result(const std::vector<double>& times) -> benchmark_result {
        auto [mean, stddev] = calculate_mean_stddev(times);
        double median = calculate_median(times);
        return {.mean_time_ms = mean,.median_time_ms = median,
            .stddev_ms = stddev,.cv_percent = (stddev / mean) * one_hundred,
            .throughput_ops_sec = (static_cast<double>(num_operations) * one_thousand) / mean,
            .raw_times = times};
    }

    void print_statistical_summary(const std::string& queue_name, const benchmark_result& result) {
        std::print("\n--- {} Statistics ---\n", queue_name);
        std::print("Mean: {:.2f} ms\n", result.mean_time_ms);
        std::print("Median: {:.2f} ms\n", result.median_time_ms);
        std::print("StdDev: {:.2f} ms\n", result.stddev_ms);
        std::print("Coefficient of Variation: {:.2f}%\n", result.cv_percent);
        std::print("Throughput: {:.2f} ops/sec\n", result.throughput_ops_sec);
        std::print("---------------------------\n\n");
    }

    // run all benchmarks
    template <typename QueueType, typename BenchmarkFunc>
    auto run_benchmark_suite(const std::string& queue_name,
                             int num_producers,
                             int num_consumers,
                             BenchmarkFunc benchmark_function) -> benchmark_result {
        perform_warmup_runs(queue_name, num_producers, num_consumers, benchmark_function);
        auto times = collect_timing_data(queue_name, num_producers, num_consumers, benchmark_function);
        auto result = create_benchmark_result(times);
        print_statistical_summary(queue_name, result);
        return result;
    }

    void print_benchmark_header(int num_producers, int num_consumers) {
        std::print("\n=====================================================\n");
        std::print("Running benchmark with {} producers and {} consumers\n", num_producers, num_consumers);
        std::print("=====================================================\n");
    }
}
namespace{
// Benchmark runners for specific configurations
 auto run_omp_stdthread_benchmark(int num_producers, int num_consumers)
    -> benchmark_result {
    return run_benchmark_suite<concurrent_queue_wrapper>("omp_queue + std::thread", num_producers, num_consumers, benchmark_stdthread);
}

auto run_omp_openmp_benchmark(int num_producers, int num_consumers)
    -> benchmark_result {
    return run_benchmark_suite<concurrent_queue_wrapper>("omp_queue + OpenMP", num_producers, num_consumers, benchmark_openmp);
}

auto run_concurrent_stdthread_benchmark(int num_producers,
                                               int num_consumers)
    -> benchmark_result {
    return run_benchmark_suite<concurrent_queue_wrapper>("ConcurrentQueue + std::thread", num_producers, num_consumers, benchmark_stdthread);
}

auto run_concurrent_openmp_benchmark(int num_producers,
                                            int num_consumers)
    -> benchmark_result {
    return run_benchmark_suite<concurrent_queue_wrapper>("ConcurrentQueue + OpenMP", num_producers, num_consumers, benchmark_openmp);
}
}
// Aggregate all results
struct all_benchmark_results {
    benchmark_result omp_stdthread;
    benchmark_result omp_openmp;
    benchmark_result concurrent_stdthread;
    benchmark_result concurrent_openmp;
};
namespace{
auto run_all_benchmarks(int num_producers, int num_consumers)
    -> all_benchmark_results {
    return {
        run_omp_stdthread_benchmark(num_producers, num_consumers),
        run_omp_openmp_benchmark(num_producers, num_consumers),
        run_concurrent_stdthread_benchmark(num_producers, num_consumers),
        run_concurrent_openmp_benchmark(num_producers, num_consumers)
    };
}

void print_comparison_header(int num_producers, int num_consumers) {
    std::print("\n--- COMPARISON SUMMARY ---\n");
    std::print("Configuration: {} producers, {} consumers\n", num_producers, num_consumers);
    std::print("{:<30}{:<15}{:<15}{:<15}\n", "Implementation", "Mean Time (ms)", "Throughput", "CV (%)");
    std::print("{}\n", std::string(75, '-'));
}

void print_result_row(const std::string& name,
                             const benchmark_result& result) {
    std::print("{:<30}{:<15.2f}{:<15.2f}{:<15.2f}\n",
               name, result.mean_time_ms, result.throughput_ops_sec, result.cv_percent);
}

void print_comparison_table(const all_benchmark_results& results) {
    print_result_row("omp_queue + std::thread", results.omp_stdthread);
    print_result_row("omp_queue + OpenMP", results.omp_openmp);
    print_result_row("ConcurrentQueue + std::thread", results.concurrent_stdthread);
    print_result_row("ConcurrentQueue + OpenMP", results.concurrent_openmp);
    std::print("\n---------------------------\n\n");
}

void print_relative_performance(const all_benchmark_results& results) {
    double base_time = results.omp_stdthread.mean_time_ms;
    std::print("Relative Performance (lower is better, omp_queue + std::thread = 1.0):\n");
    std::print("omp_queue + OpenMP: {:.2f}x\n", results.omp_openmp.mean_time_ms / base_time);
    std::print("ConcurrentQueue + std::thread: {:.2f}x\n", results.concurrent_stdthread.mean_time_ms / base_time);
    std::print("ConcurrentQueue + OpenMP: {:.2f}x\n", results.concurrent_openmp.mean_time_ms / base_time);
    std::print("\n=====================================================\n");
}

// Run the full comparison
void run_full_comparison(int num_producers, int num_consumers) {
    print_benchmark_header(num_producers, num_consumers);
    auto results = run_all_benchmarks(num_producers, num_consumers);
    print_comparison_header(num_producers, num_consumers);
    print_comparison_table(results);
    print_relative_performance(results);
}
}