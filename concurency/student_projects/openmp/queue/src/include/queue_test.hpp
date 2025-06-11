#pragma once

#include <string>
#include <vector>

// Structure holding detailed benchmark results including timing statistics and
// throughput.
struct benchmark_result {
    double mean_time_ms;    // Average elapsed time in milliseconds
    double median_time_ms;  // Median elapsed time in milliseconds
    double stddev_ms;   // Standard deviation of elapsed times in milliseconds
    double cv_percent;  // Coefficient of variation in percent
    double throughput_ops_sec;  // Throughput measured as operations per second
    std::vector<double> raw_times;  // Raw individual timing measurements
};

// Benchmark implementation using std::jthread for concurrency.
// Returns the average time taken to complete the benchmark in milliseconds.
template <typename QueueType>
auto static benchmark_stdjthread(const std::string& queue_name,
                                 int num_producers,
                                 int num_consumers,
                                 bool warmup = false) -> double;

// Benchmark implementation using OpenMP for concurrency.
// Returns the average time taken to complete the benchmark in milliseconds.
template <typename QueueType>
auto static benchmark_openmp(const std::string& queue_name,
                             int num_producers,
                             int num_consumers,
                             bool warmup = false) -> double;

// Runs a full benchmark suite using the provided benchmark function.
// Collects detailed statistics and returns them as benchmark_result.
template <typename QueueType, typename BenchmarkFunc>
auto static run_benchmark_suite(const std::string& queue_name,
                                int num_producers,
                                int num_consumers,
                                BenchmarkFunc benchmark_function)
    -> benchmark_result;

// Runs a complete comparison of all queue implementations and benchmark methods
// using the specified numbers of producers and consumers.
auto run_full_comparison(int num_producers, int num_consumers) -> void;
