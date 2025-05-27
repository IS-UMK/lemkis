#include "../include/omp_queue.hpp"

#include <omp.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <functional>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <string>
#include <thread>
#include <vector>
#include <print>

#include "../include/concurrent_queue.hpp"

// Benchmark configuration
constexpr int num_operations = 1000000;    // Number of operations per test
constexpr int warmup_iterations = 2;       // Number of warm-up iterations
constexpr int measurement_iterations = 5;  // Number of measurement iterations

// Synchronization barrier for std::thread tests
std::atomic<int> barrier(0);

// Wrapper for concurrent_queue to standardize interface
class concurrent_queue_wrapper {
  private:
    concurrent_queue<int> queue;

  public:
    void push(const int& item) { queue.push(item); }

    auto pop(int& result) -> bool {
        if (queue.try_peek(result)) { return queue.try_pop(); }
        return false;
    }

    [[nodiscard]] auto empty() const -> bool { return queue.empty(); }

    [[nodiscard]] auto size() const -> std::size_t { return queue.size(); }
};

// Result structure to store benchmark data
struct benchmark_result {
    double mean_time_ms;
    double median_time_ms;
    double stddev_ms;
    double cv_percent;
    double throughput_ops_sec;
    std::vector<double> raw_times;
};

// Function to run a single benchmark iteration using std::thread
template <typename QueueType>
auto benchmark_stdthread(const std::string& queue_name,
                         int num_producers,
                         int num_consumers,
                         bool warmup = false) -> double {
    QueueType queue;
    std::atomic<int> items_produced(0);
    std::atomic<int> items_consumed(0);
    std::vector<std::thread> threads;
    barrier.store(0);

    // Calculate workload per producer
    int const items_per_producer = num_operations / num_producers;
    int total_items = items_per_producer * num_producers;

    auto start_time = std::chrono::high_resolution_clock::now();

    // Create producer threads
    for (int i = 0; i < num_producers; i++) {
        threads.emplace_back([&, i]() {
            // Synchronize thread start
            barrier.fetch_add(1);
            while (barrier.load() < num_producers + num_consumers) {
                std::this_thread::yield();
            }

            // Produce items
            for (int j = 0; j < items_per_producer; j++) {
                queue.push(i * items_per_producer + j);
                items_produced.fetch_add(1);
            }
        });
    }

    // Create consumer threads
    for (int i = 0; i < num_consumers; i++) {
        threads.emplace_back([&]() {
            // Synchronize thread start
            barrier.fetch_add(1);
            while (barrier.load() < num_producers + num_consumers) {
                std::this_thread::yield();
            }

            // Consume items
            int value;
            while (items_consumed.load() < total_items) {
                if (queue.pop(value)) {
                    items_consumed.fetch_add(1);
                } else {
                    if (items_produced.load() == total_items && queue.empty()) {
                        // Double-check to prevent race conditions
                        if (items_consumed.load() >= total_items) { break; }
                    }
                    std::this_thread::yield();
                }
            }
        });
    }

    // Join all threads
    for (auto& t : threads) {
        if (t.joinable()) { t.join(); }
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    double elapsed_ms =
        std::chrono::duration<double, std::milli>(end_time - start_time)
            .count();

    // Only print results if this isn't a warmup run
    if (!warmup) {
        std::print("{:<25} | P: {:>2} | C: {:>2} | Time: {:.2f} ms | Throughput: {:.2f} ops/sec\n",
                   queue_name, num_producers, num_consumers, elapsed_ms, (total_items * 1000.0 / elapsed_ms));
    }

    return elapsed_ms;
}

// Function to run a single benchmark iteration using OpenMP
template <typename QueueType>
auto benchmark_openmp(const std::string& queue_name,
                      int num_producers,
                      int num_consumers,
                      bool warmup = false) -> double {
    QueueType queue;
    int items_produced = 0;
    int items_consumed = 0;

    // Calculate workload per producer
    int const items_per_producer = num_operations / num_producers;
    int const total_items = items_per_producer * num_producers;

    auto start_time = std::chrono::high_resolution_clock::now();

#pragma omp parallel num_threads(num_producers + num_consumers)
    {
        int thread_id = omp_get_thread_num();

        if (thread_id < num_producers) {
            // Producer thread
            for (int j = 0; j < items_per_producer; j++) {
                queue.push(thread_id * items_per_producer + j);
#pragma omp atomic
                items_produced++;
            }
        } else {
            // Consumer thread
            int value;
            while (true) {
                int current_consumed;
                int current_produced;

#pragma omp atomic read
                current_consumed = items_consumed;

                if (current_consumed >= total_items) { break; }

                if (queue.pop(value)) {
#pragma omp atomic
                    items_consumed++;
                } else {
#pragma omp atomic read
                    current_produced = items_produced;

                    if (current_produced == total_items && queue.empty()) {
#pragma omp atomic read
                        current_consumed = items_consumed;

                        if (current_consumed >= total_items) { break; }
                    }
#pragma omp taskyield
                }
            }
        }
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    double elapsed_ms =
        std::chrono::duration<double, std::milli>(end_time - start_time)
            .count();

    // Only print results if this isn't a warmup run
    if (!warmup) {
        std::print("{:<25} | P: {:>2} | C: {:>2} | Time: {:.2f} ms | Throughput: {:.2f} ops/sec\n",
                   queue_name, num_producers, num_consumers, elapsed_ms, (total_items * 1000.0 / elapsed_ms));
    }

    return elapsed_ms;
}

// Function to run multiple iterations and calculate statistics
template <typename QueueType, typename BenchmarkFunc>
benchmark_result run_benchmark_suite(const std::string& queue_name,
                                     int num_producers,
                                     int num_consumers,
                                     BenchmarkFunc benchmark_function) {
    std::vector<double> times;

    // Warm-up runs
    for (int i = 0; i < warmup_iterations; i++) {
        benchmark_function(
            queue_name + " (warmup)", num_producers, num_consumers, true);
    }

    // Measurement runs
    for (int i = 0; i < measurement_iterations; i++) {
        double time = benchmark_function(
            queue_name + " (run " + std::to_string(i + 1) + ")",
            num_producers,
            num_consumers,
            false);
        times.push_back(time);
    }

    // Calculate statistics
    double sum = std::accumulate(times.begin(), times.end(), 0.0);
    double mean = sum / times.size();

    std::vector<double> diff(times.size());
    std::transform(times.begin(), times.end(), diff.begin(), [mean](double x) {
        return x - mean;
    });
    double sq_sum =
        std::inner_product(diff.begin(), diff.end(), diff.begin(), 0.0);
    double stddev = std::sqrt(sq_sum / times.size());

    // Sort times for percentiles
    std::vector<double> sorted_times = times;
    std::sort(sorted_times.begin(), sorted_times.end());
    double median = sorted_times[sorted_times.size() / 2];
    double cv_percent = (stddev / mean) * 100;
    double throughput = (num_operations * 1000.0 / mean);

    benchmark_result result;
    result.mean_time_ms = mean;
    result.median_time_ms = median;
    result.stddev_ms = stddev;
    result.cv_percent = cv_percent;
    result.throughput_ops_sec = throughput;
    result.raw_times = times;

    // Print statistics
    std::print("\n--- {} Statistics ---\n", queue_name);
    std::print("Mean: {:.2f} ms\n", mean);
    std::print("Median: {:.2f} ms\n", median);
    std::print("StdDev: {:.2f} ms\n", stddev);
    std::print("Coefficient of Variation: {:.2f}%\n", cv_percent);
    std::print("Throughput: {:.2f} ops/sec\n", throughput);
    std::print("---------------------------\n\n");

    return result;
}

// Function to run a full comparison and print a summary
void run_full_comparison(int num_producers, int num_consumers) {
    std::print("\n=====================================================\n");
    std::print("Running benchmark with {} producers and {} consumers\n", num_producers, num_consumers);
    std::print("=====================================================\n");

    // Run all four combinations
    auto result1 =
        run_benchmark_suite<omp_queue<int>>("omp_queue + std::thread",
                                           num_producers,
                                           num_consumers,
                                           benchmark_stdthread<omp_queue<int>>);

    auto result2 =
        run_benchmark_suite<omp_queue<int>>("omp_queue + OpenMP",
                                           num_producers,
                                           num_consumers,
                                           benchmark_openmp<omp_queue<int>>);

    auto result3 = run_benchmark_suite<concurrent_queue_wrapper>(
        "ConcurrentQueue + std::thread",
        num_producers,
        num_consumers,
        benchmark_stdthread<concurrent_queue_wrapper>);

    auto result4 = run_benchmark_suite<concurrent_queue_wrapper>(
        "ConcurrentQueue + OpenMP",
        num_producers,
        num_consumers,
        benchmark_openmp<concurrent_queue_wrapper>);

    // Print comparison table
    std::print("\n--- COMPARISON SUMMARY ---\n");
    std::print("Configuration: {} producers, {} consumers\n", num_producers, num_consumers);
    std::print("{:<30}{:<15}{:<15}{:<15}\n", "Implementation", "Mean Time (ms)", "Throughput", "CV (%)");
    std::print("{}\n", std::string(75, '-'));

    std::print("{:<30}{:<15.2f}{:<15.2f}{:<15.2f}\n", "omp_queue + std::thread",
               result1.mean_time_ms, result1.throughput_ops_sec, result1.cv_percent);

    std::print("{:<30}{:<15.2f}{:<15.2f}{:<15.2f}\n", "omp_queue + OpenMP",
               result2.mean_time_ms, result2.throughput_ops_sec, result2.cv_percent);

    std::print("{:<30}{:<15.2f}{:<15.2f}{:<15.2f}\n", "ConcurrentQueue + std::thread",
               result3.mean_time_ms, result3.throughput_ops_sec, result3.cv_percent);

    std::print("{:<30}{:<15.2f}{:<15.2f}{:<15.2f}\n", "ConcurrentQueue + OpenMP",
               result4.mean_time_ms, result4.throughput_ops_sec, result4.cv_percent);

    std::print("\n---------------------------\n\n");

    // Calculate relative performance
    double baseline = result1.mean_time_ms;
    std::print("Relative Performance (lower is better, omp_queue + std::thread = 1.0):\n");
    std::print("omp_queue + OpenMP: {:.2f}x\n", result2.mean_time_ms / baseline);
    std::print("ConcurrentQueue + std::thread: {:.2f}x\n", result3.mean_time_ms / baseline);
    std::print("ConcurrentQueue + OpenMP: {:.2f}x\n", result4.mean_time_ms / baseline);

    std::print("\n=====================================================\n");
}