#pragma once

#include <string>
#include <vector>
// Benchmark configuration

// Struktura wyników benchmarku
struct benchmark_result {
    double mean_time_ms;
    double median_time_ms;
    double stddev_ms;
    double cv_percent;
    double throughput_ops_sec;
    std::vector<double> raw_times;
};

// Benchmark z użyciem std::thread
template <typename QueueType>
auto static benchmark_stdjthread(const std::string& queue_name,
                                 int num_producers,
                                 int num_consumers,
                                 bool warmup = false) -> double;

// Benchmark z użyciem OpenMP
template <typename QueueType>
auto static benchmark_openmp(const std::string& queue_name,
                             int num_producers,
                             int num_consumers,
                             bool warmup = false) -> double;

// Uruchomienie serii benchmarków i zebranie statystyk
template <typename QueueType, typename BenchmarkFunc>
auto static run_benchmark_suite(const std::string& queue_name,
                                int num_producers,
                                int num_consumers,
                                BenchmarkFunc benchmark_function)
    -> benchmark_result;

// Porównanie pełne — uruchamia wszystkie kombinacje kolejek i metod
auto run_full_comparison(int num_producers, int num_consumers) -> void;