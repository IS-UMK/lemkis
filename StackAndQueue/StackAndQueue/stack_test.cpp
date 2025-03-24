#pragma once

#include "stack_test.hpp"
#include "stack.hpp"
#include "con_stack.hpp"

#include <print>
#include <iostream>
#include <thread>
#include <vector>
#include <atomic>
#include <chrono>

// Demo of data race with the original Stack
auto StackTest::demonstrate_data_race() -> void {
    std::println("=== Demonstrating Data Race Issues ===");

    Stack<int> unsafe_stack;
    std::atomic<bool> stop_flag(false);
    std::atomic<int> producer_count(0);
    std::atomic<int> consumer_count(0);
    std::atomic<bool> race_detected(false);

    // Producer thread
    std::thread producer([&]() {
        for (int i = 0; i < 10000 && !race_detected; ++i) {
            unsafe_stack.push(i);
            producer_count++;
            // Yield to increase chance of race condition
            std::this_thread::yield();
        }
        });

    // Consumer thread
    std::thread consumer([&]() {
        for (int i = 0; i < 10000 && !race_detected; ++i) {
            // This can crash if stack becomes empty between empty() check and pop()
            std::println("got here"); //zakomentuj to zeby wywolac race condition
            try {
                if (!unsafe_stack.empty()) {
                    int value = unsafe_stack.top(); // Potential race
                    unsafe_stack.pop();             // Potential race
                    consumer_count++;
                }
            }
            catch (const std::exception& e) {
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
    }
    else {
        std::cout << "No race condition detected in this run, but the code is "
            "still unsafe."
            << std::endl;
        std::cout << "The absence of a detected race doesn't mean the code is "
            "thread-safe."
            << std::endl;
    }
}

// Demo of thread-safe stack
auto StackTest::demonstrate_concurrent_stack() -> void {
    std::cout << "\n=== Demonstrating Thread-Safe Stack ===" << std::endl;

    ConcurrentStack<int> safe_stack;
    std::atomic<bool> stop_flag(false);
    std::atomic<int> producer_count(0);
    std::atomic<int> consumer_count(0);

    // Producer thread
    std::thread producer([&]() {
        for (int i = 0; i < 10000 && !stop_flag; ++i) {
            safe_stack.push(i);
            producer_count++;
            // Yield to increase chance of finding issues if they exist
            std::this_thread::yield();
        }
        });

    // Consumer thread with blocking operation
    std::thread consumer([&]() {
        while (!stop_flag || !safe_stack.empty()) {
            int value;
            if (safe_stack.try_pop(value)) { consumer_count++; }
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
    std::cout << "Items left in stack: " << safe_stack.size() << std::endl;
    std::cout
        << "The concurrent implementation handles multiple threads correctly."
        << std::endl;
}

// Demo of condition variable for efficient waiting
auto StackTest::demonstrate_condition_variable() -> void {
    std::cout << "\n=== Demonstrating Condition Variable Usage ==="
        << std::endl;

    ConcurrentStack<int> safe_stack;
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
            safe_stack.push(i);
            producer_count++;
        }
        });

    // Consumer thread with blocking wait
    std::thread consumer([&]() {
        while (!stop_flag || !safe_stack.empty()) {
            int value;
            // Blocking pop with condition variable
            safe_stack.pop(value);
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

auto StackTest::stackTest() -> void {
    StackTest::demonstrate_data_race();
    StackTest::demonstrate_concurrent_stack();
    StackTest::demonstrate_condition_variable();
}