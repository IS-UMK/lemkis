#include "include/stack_test.hpp"

#include <atomic>
#include <chrono>
#include <iostream>
#include <print>
#include <thread>
#include <vector>

#include "include/con_stack.hpp"
#include "include/stack.hpp"

// Demonstration in which use of unsafe methods leads to data race
auto StackTest::demonstrate_data_race() -> void {
    std::println("=== Demonstrating Data Race Issues ===");
    ConcurrentStack<int> unsafe_stack;
    std::atomic<bool> stop_flag(false);
    std::atomic<int> producer_count(0);
    std::atomic<int> consumer_count(0);
    std::atomic<bool> race_detected(false);
    std::jthread producer([&](std::stop_token stoken) {
        for (int i = 0; i < 10000 && !stoken.stop_requested() && !race_detected;
             ++i) {
            unsafe_stack.unsafe_push(i);
            producer_count++;
            std::this_thread::yield();
        }
    });
    std::jthread consumer([&](std::stop_token stoken) {
        for (int i = 0; i < 10000 && !stoken.stop_requested() && !race_detected;
             ++i) {
            // std::println("got here");
            try {
                if (!unsafe_stack.unsafe_empty()) {
                    int value = unsafe_stack.unsafe_top();
                    unsafe_stack.unsafe_pop();
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
    std::this_thread::sleep_for(std::chrono::seconds(2));
    producer.request_stop();
    consumer.request_stop();
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

// Demonstration where mutex is used to ensure safe concurrency
auto StackTest::demonstrate_concurrent_stack() -> void {
    std::cout << "\n=== Demonstrating Thread-Safe Stack ===" << std::endl;
    ConcurrentStack<int> safe_stack;
    std::atomic<bool> stop_flag(false);
    std::atomic<int> producer_count(0);
    std::atomic<int> consumer_count(0);
    std::jthread producer([&](std::stop_token stoken) {
        for (int i = 0; i < 10000 && !stoken.stop_requested(); ++i) {
            safe_stack.push(i);
            producer_count++;
            std::this_thread::yield();
        }
    });
    std::jthread consumer([&](std::stop_token stoken) {
        while (!stoken.stop_requested() || !safe_stack.empty()) {
            int value;
            if (safe_stack.try_pop(value)) { consumer_count++; }
            std::this_thread::yield();
        }
    });
    std::this_thread::sleep_for(std::chrono::seconds(2));
    producer.request_stop();
    consumer.request_stop();
    std::cout << "Producer pushed: " << producer_count << " items" << std::endl;
    std::cout << "Consumer popped: " << consumer_count << " items" << std::endl;
    // std::cout << "Items left in stack: " << safe_stack.size() << std::endl;
    std::cout
        << "The concurrent implementation handles multiple threads correctly."
        << std::endl;
}

// Demonstration where Producer thread occasionally pauses and Consumer thread
// waits based on cv
auto StackTest::demonstrate_condition_variable() -> void {
    std::cout << "\n=== Demonstrating Condition Variable Usage ==="
              << std::endl;
    ConcurrentStack<int> safe_stack;
    std::atomic<bool> stop_flag(false);
    std::atomic<int> producer_count(0);
    std::atomic<int> consumer_count(0);
    std::jthread producer([&](std::stop_token stoken) {
        for (int i = 0; i < 100 && !stoken.stop_requested(); ++i) {
            if (i % 10 == 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            safe_stack.push(i);
            producer_count++;
        }
    });
    std::jthread consumer([&](std::stop_token stoken) {
        while (!stoken.stop_requested() || !safe_stack.empty()) {
            safe_stack.pop();
            consumer_count++;
        }
    });
    std::this_thread::sleep_for(std::chrono::seconds(1));
    producer.request_stop();
    consumer.request_stop();
    std::cout << "Producer pushed: " << producer_count << " items" << std::endl;
    std::cout << "Consumer popped: " << consumer_count << " items" << std::endl;
    std::cout << "The condition variable allows efficient waiting without "
                 "busy-waiting."
              << std::endl;
}

auto StackTest::stackTest() -> void {
    // StackTest::demonstrate_data_race();
    StackTest::demonstrate_concurrent_stack();
    StackTest::demonstrate_condition_variable();
}