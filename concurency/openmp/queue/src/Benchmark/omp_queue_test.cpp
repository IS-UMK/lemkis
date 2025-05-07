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

constexpr int NUM_OPERATIONS = 1000000;    // Number of operations per test
constexpr int WARMUP_ITERATIONS = 2;       // Number of warm-up iterations
constexpr int MEASUREMENT_ITERATIONS = 5;  // Number of measurement iterations

// Wrapper for concurrent_queue to standardize interface
class ConcurrentQueueWrapper {
  private:
    concurrent_queue<int> queue;

  public:
    void push(const int& item) { queue.push(item); }

    bool pop(int& result) {
        if (queue.try_peek(result)) { return queue.try_pop(); }
        return false;
    }

    bool empty() const { return queue.empty(); }

    std::size_t size() const { return queue.size(); }
};

// Result structure to store benchmark data
struct BenchmarkResult {
    double mean_time_ms;
    double median_time_ms;
    double stddev_ms;
    double cv_percent;
    double throughput_ops_sec;
    std::vector<double> raw_times;
};

// Function to run a single benchmark iteration using std::jthread
template <typename QueueType>
double benchmark_stdjthread(const std::string& queue_name,
                            int num_producers,
                            int num_consumers,
                            bool warmup = false) {
    QueueType queue;
    std::atomic<int> items_produced(0);
    std::atomic<int> items_consumed(0);
    std::atomic<bool> production_complete(false);
    std::vector<std::jthread> threads;

    int items_per_producer = NUM_OPERATIONS / num_producers;
    int total_items = items_per_producer * num_producers;

    auto start_time = std::chrono::high_resolution_clock::now();

    // Create producer threads
    for (int i = 0; i < num_producers; i++) {
        threads.emplace_back([&, i](std::stop_token) {
            for (int j = 0; j < items_per_producer; j++) {
                queue.push(i * items_per_producer + j);
                items_produced.fetch_add(1);
            }
        });
    }

    // Create consumer threads
    for (int i = 0; i < num_consumers; i++) {
        threads.emplace_back([&](std::stop_token) {
            int value;
            while (items_consumed.load() < total_items) {
                if (queue.pop(value)) {
                    items_consumed.fetch_add(1);
                } else {
                    if (items_produced.load() == total_items && queue.empty()) {
                        if (items_consumed.load() >= total_items) break;
                    }
                    std::this_thread::yield();
                }
            }
        });
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    double elapsed_ms =
        std::chrono::duration<double, std::milli>(end_time - start_time)
            .count();

    if (!warmup) {
        std::print(
            "{:<25} | P: {:>2} | C: {:>2} | Time: {:.2f} ms | Throughput: "
            "{:.2f} ops/sec\n",
            queue_name,
            num_producers,
            num_consumers,
            elapsed_ms,
            (total_items * 1000.0 / elapsed_ms));
    }

    return elapsed_ms;
}


// Function to run a single benchmark iteration using OpenMP
template <typename QueueType>
double benchmark_openmp(const std::string& queue_name,
                        int num_producers,
                        int num_consumers,
                        bool warmup = false) {
    QueueType queue;
    int items_produced = 0;
    int items_consumed = 0;

    // Calculate workload per producer
    int items_per_producer = NUM_OPERATIONS / num_producers;
    int total_items = items_per_producer * num_producers;

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
        std::print(
            "{:<25} | P: {:>2} | C: {:>2} | Time: {:.2f} ms | Throughput: "
            "{:.2f} ops/sec\n",
            queue_name,
            num_producers,
            num_consumers,
            elapsed_ms,
            (total_items * 1000.0 / elapsed_ms));

    }

    return elapsed_ms;
}

// Function to run multiple iterations and calculate statistics
template <typename QueueType, typename BenchmarkFunc>
BenchmarkResult run_benchmark_suite(const std::string& queue_name,
                                    int num_producers,
                                    int num_consumers,
                                    BenchmarkFunc benchmark_function) {
    std::vector<double> times;

    // Warm-up runs
    for (int i = 0; i < WARMUP_ITERATIONS; i++) {
        benchmark_function(std::format("{} (warmup)", queue_name),
                           num_producers,
                           num_consumers,
                           true);

    }

    // Measurement runs
    for (int i = 0; i < MEASUREMENT_ITERATIONS; i++) {
        double time = benchmark_function(
            std::format("{} (run {})", queue_name, i + 1),
            num_producers,
            num_consumers, false);
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
    double throughput = (NUM_OPERATIONS * 1000.0 / mean);

    BenchmarkResult result;
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
    std::print("Running benchmark with {} producers and {} consumers\n",
               num_producers,
               num_consumers);
    std::print("=====================================================\n");
    // Run all four combinations
    auto result1 =
        run_benchmark_suite<OMPQueue<int>>("OMPQueue + std::jthread",
                                           num_producers,
                                           num_consumers,
                                           benchmark_stdjthread<OMPQueue<int>>);
    auto result2 =
        run_benchmark_suite<OMPQueue<int>>("OMPQueue + OpenMP",
                                           num_producers,
                                           num_consumers,
                                           benchmark_openmp<OMPQueue<int>>);
    auto result3 = run_benchmark_suite<ConcurrentQueueWrapper>(
        "ConcurrentQueue + std::jthread",
        num_producers,
        num_consumers,
        benchmark_stdjthread<ConcurrentQueueWrapper>);
    auto result4 = run_benchmark_suite<ConcurrentQueueWrapper>(
        "ConcurrentQueue + OpenMP",
        num_producers,
        num_consumers,
        benchmark_openmp<ConcurrentQueueWrapper>);
    // Print comparison table
    std::print("\n--- COMPARISON SUMMARY ---\n");
    std::print("Configuration: {} producers, {} consumers\n",
               num_producers,
               num_consumers);
    std::print("{:<32}{:<15}{:<15}{:<15}\n",
               "Implementation",
               "Mean Time (ms)",
               "Throughput",
               "CV (%)");
    std::print("{:-<75}\n", "");
    std::print("{:<32}{:<15.2f}{:<15.2f}{:<15.2f}\n",
               "OMPQueue + std::jthread",
               result1.mean_time_ms,
               result1.throughput_ops_sec,
               result1.cv_percent);
    std::print("{:<32}{:<15.2f}{:<15.2f}{:<15.2f}\n",
               "OMPQueue + OpenMP",
               result2.mean_time_ms,
               result2.throughput_ops_sec,
               result2.cv_percent);
    std::print("{:<32}{:<15.2f}{:<15.2f}{:<15.2f}\n",
               "ConcurrentQueue + std::jthread",
               result3.mean_time_ms,
               result3.throughput_ops_sec,
               result3.cv_percent);
    std::print("{:<32}{:<15.2f}{:<15.2f}{:<15.2f}\n",
               "ConcurrentQueue + OpenMP",
               result4.mean_time_ms,
               result4.throughput_ops_sec,
               result4.cv_percent);
    std::print("\n---------------------------\n");
    double baseline = result1.mean_time_ms;
    std::print(
        "Relative Performance (lower is better, OMPQueue + std::jthread = "
        "1.0):\n");
    std::print("OMPQueue + OpenMP: {:.2f}x\n", result2.mean_time_ms / baseline);
    std::print("ConcurrentQueue + std::jthread: {:.2f}x\n",
               result3.mean_time_ms / baseline);
    std::print("ConcurrentQueue + OpenMP: {:.2f}x\n",
               result4.mean_time_ms / baseline);
    std::print("\n=====================================================\n");
}