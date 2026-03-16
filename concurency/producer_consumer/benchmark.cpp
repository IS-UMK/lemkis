// Prosty benchmark SPSC dla różnych implementacji bufora.

#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <optional>
#include <string>
#include <thread>
#include <vector>

#include "bounded_buffer.hpp"
#include "ring_buffer.hpp"

// Lock-free SPSC ring buffer (kopiowany z notatek).
template<typename T>
class spsc_ring_buffer {
    std::vector<T> data_;
    const size_t capacity_;

    // Single Producer, Single Consumer – każdy atomic ma jednego pisarza.
    alignas(64) std::atomic<size_t> head_{0};
    alignas(64) std::atomic<size_t> tail_{0};

public:
    explicit spsc_ring_buffer(size_t capacity)
        : data_(capacity), capacity_(capacity) {}

    // Wywoływany tylko przez producenta.
    bool try_push(const T& item) {
        size_t head = head_.load(std::memory_order_relaxed);
        size_t next = (head + 1) % capacity_;

        if (next == tail_.load(std::memory_order_acquire)) {
            return false;  // pełny
        }

        data_[head] = item;
        head_.store(next, std::memory_order_release);
        return true;
    }

    // Wywoływany tylko przez konsumenta.
    std::optional<T> try_pop() {
        size_t tail = tail_.load(std::memory_order_relaxed);

        if (tail == head_.load(std::memory_order_acquire)) {
            return std::nullopt;  // pusty
        }

        T item = std::move(data_[tail]);
        tail_.store((tail + 1) % capacity_, std::memory_order_release);
        return item;
    }
};

// Pomocnicza funkcja do mierzenia czasu jednego przebiegu.
template<typename Producer, typename Consumer>
void run_benchmark(const std::string& name,
                   std::uint64_t items,
                   Producer make_producer,
                   Consumer make_consumer) {
    std::cout << "=== " << name << " — " << items << " elementów (SPSC) ===\n";

    auto start = std::chrono::steady_clock::now();

    std::thread producer = make_producer();
    std::thread consumer = make_consumer();

    producer.join();
    consumer.join();

    auto end = std::chrono::steady_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    double secs = ms / 1000.0;
    double throughput = secs > 0.0 ? items / secs : 0.0;

    std::cout << "Czas: " << ms << " ms, przepustowość: "
              << std::fixed << std::setprecision(2)
              << throughput << " elem/s\n\n";
}

int main(int argc, char** argv) {
    std::string which = (argc > 1) ? argv[1] : "all";
    std::uint64_t items = (argc > 2) ? std::strtoull(argv[2], nullptr, 10)
                                     : 1'000'000ULL;
    std::size_t capacity = (argc > 3) ? static_cast<std::size_t>(std::strtoull(argv[3], nullptr, 10))
                                      : 1024;

    if (items == 0) {
        std::cerr << "Liczba elementów musi być > 0\n";
        return 1;
    }

    if (capacity < 2) {
        std::cerr << "Pojemność bufora musi być >= 2\n";
        return 1;
    }

    std::cout << "Konfiguracja: items=" << items
              << ", capacity=" << capacity << "\n\n";

    // 1) bounded_buffer (mutex + condition_variable + std::queue)
    if (which == "all" || which == "bounded") {
        bounded_buffer<int> buf(capacity);
        std::atomic<std::uint64_t> produced{0};
        std::atomic<std::uint64_t> consumed{0};

        run_benchmark("bounded_buffer (mutex + CV)", items,
            [&]() {
                return std::thread([&]() {
                    for (std::uint64_t i = 0; i < items; ++i) {
                        buf.push(static_cast<int>(i));
                        ++produced;
                    }
                });
            },
            [&]() {
                return std::thread([&]() {
                    for (std::uint64_t i = 0; i < items; ++i) {
                        int v = buf.pop();
                        (void)v;
                        ++consumed;
                    }
                });
            }
        );

        if (produced != consumed || produced != items) {
            std::cerr << "BŁĄD (bounded_buffer): produced=" << produced
                      << " consumed=" << consumed << "\n";
        }
    }

    // 2) ring_buffer (mutex + CV + wektor kołowy)
    if (which == "all" || which == "ring") {
        ring_buffer<int> buf(capacity);
        std::atomic<std::uint64_t> produced{0};
        std::atomic<std::uint64_t> consumed{0};

        run_benchmark("ring_buffer (mutex + CV + vector)", items,
            [&]() {
                return std::thread([&]() {
                    for (std::uint64_t i = 0; i < items; ++i) {
                        buf.push(static_cast<int>(i));
                        ++produced;
                    }
                });
            },
            [&]() {
                return std::thread([&]() {
                    for (std::uint64_t i = 0; i < items; ++i) {
                        int v = buf.pop();
                        (void)v;
                        ++consumed;
                    }
                });
            }
        );

        if (produced != consumed || produced != items) {
            std::cerr << "BŁĄD (ring_buffer): produced=" << produced
                      << " consumed=" << consumed << "\n";
        }
    }

    // 3) spsc_ring_buffer — lock-free, try_push/try_pop z busy-wait.
    if (which == "all" || which == "spsc") {
        spsc_ring_buffer<int> buf(capacity);
        std::atomic<std::uint64_t> produced{0};
        std::atomic<std::uint64_t> consumed{0};

        run_benchmark("spsc_ring_buffer (lock-free, busy-wait)", items,
            [&]() {
                return std::thread([&]() {
                    for (std::uint64_t i = 0; i < items; ++i) {
                        int value = static_cast<int>(i);
                        // kręcimy się aż będzie miejsce
                        while (!buf.try_push(value)) {
                            // opcjonalnie: std::this_thread::yield();
                        }
                        ++produced;
                    }
                });
            },
            [&]() {
                return std::thread([&]() {
                    for (std::uint64_t i = 0; i < items; ++i) {
                        std::optional<int> v;
                        // kręcimy się aż coś będzie
                        while (!(v = buf.try_pop())) {
                            // opcjonalnie: std::this_thread::yield();
                        }
                        (void)*v;
                        ++consumed;
                    }
                });
            }
        );

        if (produced != consumed || produced != items) {
            std::cerr << "BŁĄD (spsc_ring_buffer): produced=" << produced
                      << " consumed=" << consumed << "\n";
        }
    }

    return 0;
}

