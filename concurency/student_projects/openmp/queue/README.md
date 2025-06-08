
# Project: Benchmarking Concurrent Queues and Parallel Operations in C++

## Project Goal
The goal of the project was to implement and compare different implementations of concurrent queues and benchmark the performance of parallel operations on large datasets. The project focuses on:

- Comparing a custom concurrent queue implemented using `std::jthread` and synchronization via mutexes with a simpler queue based on OpenMP.
- Benchmarking vector operations such as transformations and dot products, implemented using various parallel methods (standard C++ algorithms and OpenMP).
- Automating the build, testing, and code quality control process using tools like `clang-format` and `clang-tidy`.

## What Was Done?

### Queue Implementations
- **concurrent_queue**: A custom thread-safe queue using mutexes and condition variables, managing nodes with `std::unique_ptr` for safe memory management.
- **omp_queue**: A simpler queue wrapping `std::queue`, protected by OpenMP locks (`omp_lock_t`).

### Benchmarks

#### Concurrent Queue Benchmark
- Tests with varying numbers of producer and consumer threads.
- Measuring push/pop operation times and queue size.
- Use of a universal helper function `measure_and_print` to collect and display results.

#### Vectorized Operation Benchmark (Transform and Dot Product)
- Sequential benchmarks using `std::transform` and `std::inner_product`.
- Parallel benchmarks using standard C++ algorithms with execution policies `std::execution::par` and `par_unseq`.
- Parallel versions with OpenMP using parallel for loops and reductions.
- Tests on small (10,000 elements) and large (300 million elements) datasets.

### Automation
- Bash script for:
  - Cleaning and building the project using `cmake` and `make`.
  - Formatting code according to `clang-format`.
  - Static analysis and automatic code fixes using `clang-tidy`.

## Methods and Technologies Used

### Multithreading and Synchronization
- `std::jthread` and atomic variables for thread management and signaling.
- Mutexes and condition variables for safe queue operations in `concurrent_queue`.
- OpenMP locks and pragma directives for synchronization and parallel loops in `omp_queue` and benchmarks.

### C++ Parallel Algorithms (C++17/20)
- `std::transform` with execution policies: sequential, parallel, and parallel unsequenced.
- `std::transform_reduce` for parallel dot product computation.

### OpenMP
- Parallel loops with reduction clauses for summation.
- Thread-safe queue with OpenMP locks.

### Code Quality and Automation Tools
- `clang-format` for consistent code formatting.
- `clang-tidy` for static analysis and automatic fixes.

## Summary
The project provided practical insights into various concurrency and parallelism techniques in modern C++. The custom queue with mutexes was compared with an OpenMP-based solution, highlighting trade-offs between implementation complexity and performance. Vector operation benchmarks demonstrated the impact of different execution policies and OpenMP approaches on performance across different data sizes.

Build and code quality automation ensures ease of maintenance and future development of the project.
