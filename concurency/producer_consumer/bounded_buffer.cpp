#include "bounded_buffer.hpp"
#include <thread>
#include <vector>
#include <atomic>
#include <iostream>
#include <cassert>



int main() {
    bounded_buffer<int> bufor(5);
    std::atomic<int> suma_produced{0};
    std::atomic<int> suma_consumed{0};

    const int NUM_PRODUCERS = 3;
    const int NUM_CONSUMERS = 2;
    const int ITEMS_PER_PRODUCER = 100;

    // --- Producenci ---
    std::vector<std::thread> producers;
    for (int id = 0; id < NUM_PRODUCERS; ++id) {
        producers.emplace_back([&bufor, &suma_produced, id] {
            for (int i = 0; i < ITEMS_PER_PRODUCER; ++i) {
                int value = id * ITEMS_PER_PRODUCER + i;
                bufor.push(value);
                suma_produced += value;
            }
        });
    }

    // --- Konsumenci ---
    const int TOTAL_ITEMS = NUM_PRODUCERS * ITEMS_PER_PRODUCER;
    std::atomic<int> consumed_count{0};

    std::vector<std::thread> consumers;
    for (int id = 0; id < NUM_CONSUMERS; ++id) {
        consumers.emplace_back([&bufor, &suma_consumed,  
&consumed_count, TOTAL_ITEMS] {
            while (true) {
                int my_count = consumed_count.fetch_add(1);
                if (my_count >= TOTAL_ITEMS) break;

                int value = bufor.pop();
                suma_consumed += value;
            }
        });
    }

    for (auto& t : producers) t.join();
    for (auto& t : consumers) t.join();

    std::cout << "Suma wyprodukowana: " << suma_produced << "\n";
    std::cout << "Suma skonsumowana:  " << suma_consumed << "\n";
    assert(suma_produced == suma_consumed);
    std::cout << "✅ Wszystkie elementy przetworzone poprawnie!\n";
}