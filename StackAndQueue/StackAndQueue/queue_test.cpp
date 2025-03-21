#include "queue.hpp"
#include "con_queue.hpp"
#include <print>
#include "queue_test.hpp"
#include <iostream>
#include <thread>
#include <vector>
#include <atomic>
#include <chrono>

// Demo of data race with the original Queue
void demonstrate_data_race() {
    std::println("=== Demonstrating Data Race Issues ===");

    Queue<int> unsafe_queue;
    std::atomic<bool> stop_flag(false);
    std::atomic<int> producer_count(0);
    std::atomic<int> consumer_count(0);
    std::atomic<bool> race_detected(false);
    
    // Producer thread
    std::thread producer([&]() {
        for (int i = 0; i < 10000 && !race_detected; ++i) {
            unsafe_queue.push(i);
            producer_count++;
            // Yield to increase chance of race condition
            std::this_thread::yield();
        }
    });
    
    // Consumer thread
    std::thread consumer([&]() {
        for (int i = 0; i < 10000 && !race_detected; ++i) {
            // This can crash if queue becomes empty between empty() check and
            // pop()
            std::println("got here"); //zakomentuj to zeby wywolac race condition
            try {
                if (!unsafe_queue.empty()) {
                    int value = unsafe_queue.front();  // Potential race
                    unsafe_queue.pop();                // Potential race
                    consumer_count++;
                }
            } catch (const std::exception& e) {
                std::cout << "Race condition detected: " << e.what()
                          << std::endl;
                race_detected = true;
            }
            std::this_thread::yield();
        }
    });

    // Wait for threads to finish or timeout
    std::this_thread::sleep_for(std::chrono::seconds(2));
    stop_flag = true;

    producer.join();
    consumer.join();

    std::cout << "Producer pushed: " << producer_count << " items" << std::endl;
    std::cout << "Consumer popped: " << consumer_count << " items" << std::endl;

    if (race_detected) {
        std::cout << "A race condition was detected!" << std::endl;
    } else {
        std::cout << "No race condition detected in this run, but the code is "
                     "still unsafe."
                  << std::endl;
        std::cout << "The absence of a detected race doesn't mean the code is "
                     "thread-safe."
                  << std::endl;
    }
}

// Demo of thread-safe queue
void demonstrate_concurrent_queue() {
    std::cout << "\n=== Demonstrating Thread-Safe Queue ===" << std::endl;

    ConcurrentQueue<int> safe_queue;
    std::atomic<bool> stop_flag(false);
    std::atomic<int> producer_count(0);
    std::atomic<int> consumer_count(0);

    // Producer thread
    std::thread producer([&]() {
        for (int i = 0; i < 10000 && !stop_flag; ++i) {
            safe_queue.push(i);
            producer_count++;
            // Yield to increase chance of finding issues if they exist
            std::this_thread::yield();
        }
    });

    // Consumer thread with blocking operation
    std::thread consumer([&]() {
        while (!stop_flag || !safe_queue.empty()) {
            int value;
            if (safe_queue.try_pop(value)) { consumer_count++; }
            std::this_thread::yield();
        }
    });

    // Let them run for a bit
    std::this_thread::sleep_for(std::chrono::seconds(2));
    stop_flag = true;

    producer.join();
    consumer.join();

    std::cout << "Producer pushed: " << producer_count << " items" << std::endl;
    std::cout << "Consumer popped: " << consumer_count << " items" << std::endl;
    std::cout << "Items left in queue: " << safe_queue.size() << std::endl;
    std::cout
        << "The concurrent implementation handles multiple threads correctly."
        << std::endl;
}

// Demo of condition variable for efficient waiting
void demonstrate_condition_variable() {
    std::cout << "\n=== Demonstrating Condition Variable Usage ==="
              << std::endl;

    ConcurrentQueue<int> safe_queue;
    std::atomic<bool> stop_flag(false);
    std::atomic<int> producer_count(0);
    std::atomic<int> consumer_wait_count(0);
    std::atomic<int> consumer_count(0);

    // Producer thread that occasionally pauses
    std::thread producer([&]() {
        for (int i = 0; i < 100 && !stop_flag; ++i) {
            // Sometimes sleep to make consumer wait
            if (i % 10 == 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            safe_queue.push(i);
            producer_count++;
        }
    });

    // Consumer thread with blocking wait
    std::thread consumer([&]() {
        while (!stop_flag || !safe_queue.empty()) {
            int value;
            // Blocking pop with condition variable
            safe_queue.pop(value);
            consumer_count++;

            // Show that we got a value
            std::cout << "Consumer got value: " << value << std::endl;
        }
    });

    // Let them run for a bit
    std::this_thread::sleep_for(std::chrono::seconds(1));
    stop_flag = true;

    producer.join();
    consumer.join();

    std::cout << "Producer pushed: " << producer_count << " items" << std::endl;
    std::cout << "Consumer popped: " << consumer_count << " items" << std::endl;
    std::cout << "The condition variable allows efficient waiting without "
                 "busy-waiting."
              << std::endl;
}

auto queueTest() -> void {
    demonstrate_data_race();
    demonstrate_concurrent_queue();
    demonstrate_condition_variable();
}

