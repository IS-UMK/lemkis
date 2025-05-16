#pragma once

// Benchmark configuration

// Struktura wyników benchmarku
struct BenchmarkResult {
    double mean_time_ms;
    double median_time_ms;
    double stddev_ms;
    double cv_percent;
    double throughput_ops_sec;
    std::vector<double> raw_times;
};

// Benchmark z użyciem std::thread
template <typename QueueType>
double benchmark_stdjthread(const std::string& queue_name,
                            int num_producers,
                            int num_consumers,
                            bool warmup = false);

// Benchmark z użyciem OpenMP
template <typename QueueType>
double benchmark_openmp(const std::string& queue_name,
                        int num_producers,
                        int num_consumers,
                        bool warmup = false);

// Uruchomienie serii benchmarków i zebranie statystyk
template <typename QueueType, typename BenchmarkFunc>
BenchmarkResult run_benchmark_suite(const std::string& queue_name,
                                    int num_producers,
                                    int num_consumers,
                                    BenchmarkFunc benchmark_function);

// Porównanie pełne — uruchamia wszystkie kombinacje kolejek i metod
void run_full_comparison(int num_producers, int num_consumers);