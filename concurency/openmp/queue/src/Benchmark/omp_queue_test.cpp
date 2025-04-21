#include "../include/omp_queue.hpp"

#include <chrono>
#include <iostream>
#include <jthread>
#include <thread>
#include <vector>

#include "../include/concurrent_queue.hpp"

constexpr int item_count = 100000;

void test_no_openmp_ompqueue(int prod, int cons) {
    std::cout << "\n[OMPQueue] No OpenMP - Producers: " << prod
              << ", Consumers: " << cons << "\n";
    OMPQueue<int> q;
    int pushed = 0, popped = 0;

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < prod; ++i) {
        for (int j = 0; j < item_count / prod; ++j) {
            q.push(j + i * 10000);
            ++pushed;
        }
    }

    for (int i = 0; i < cons; ++i) {
        int local = 0, val = 0;
        while (local < item_count / cons) {
            if (q.pop(val)) {
                ++popped;
                ++local;
            }
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Pushed: " << pushed << ", Popped: " << popped
              << ", Remaining: " << q.size() << ", Time: "
              << std::chrono::duration<double>(end - start).count() << " s\n";
}

void test_openmp_ompqueue(int prod, int cons) {
    std::cout << "\n[OMPQueue] With OpenMP - Producers: " << prod
              << ", Consumers: " << cons << "\n";
    OMPQueue<int> q;
    int pushed = 0, popped = 0;

    auto start = std::chrono::high_resolution_clock::now();

#pragma omp parallel num_threads(prod + cons)
    {
        int tid = omp_get_thread_num();
        if (tid < prod) {
            for (int i = 0; i < item_count / prod; ++i) {
                q.push(i + tid * 10000);
#pragma omp atomic
                ++pushed;
            }
        } else {
            int val, local = 0;
            while (local < item_count / cons) {
                if (q.pop(val)) {
#pragma omp atomic
                    ++popped;
                    ++local;
                }
            }
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Pushed: " << pushed << ", Popped: " << popped
              << ", Remaining: " << q.size() << ", Time: "
              << std::chrono::duration<double>(end - start).count() << " s\n";
}

void test_jthread_concurrentqueue(int prod, int cons) {
    std::cout << "\n[ConcurrentQueue] With std::jthread - Producers: " << prod
              << ", Consumers: " << cons << "\n";
    concurrent_queue<int> q;
    int pushed = 0, popped = 0;
    std::mutex count_mutex;

    auto start = std::chrono::high_resolution_clock::now();

    std::vector<std::jthread> threads;

    for (int i = 0; i < prod; ++i) {
        threads.emplace_back([&q, &pushed, i, prod, &count_mutex]() {
            for (int j = 0; j < item_count / prod; ++j) {
                q.push(j + i * 10000);
                std::scoped_lock lock(count_mutex);
                ++pushed;
            }
        });
    }

    for (int i = 0; i < cons; ++i) {
        threads.emplace_back([&q, &popped, i, cons, &count_mutex]() {
            int val, local = 0;
            while (local < item_count / cons) {
                if (q.try_peek(val)) {
                    q.pop();
                    std::scoped_lock lock(count_mutex);
                    ++popped;
                    ++local;
                }
            }
        });
    }

    for (auto& t : threads) t.join();

    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Pushed: " << pushed << ", Popped: " << popped
              << ", Remaining: " << q.size() << ", Time: "
              << std::chrono::duration<double>(end - start).count() << " s\n";
}

int main() {
    std::vector<std::pair<int, int>> configs = {{1, 1}, {2, 2}, {4, 4}};

    for (const auto& [prod, cons] : configs) {
        test_no_openmp_ompqueue(prod, cons);
        test_openmp_ompqueue(prod, cons);
        test_jthread_concurrentqueue(prod, cons);
    }

    return 0;
}
