#pragma once

#include "stack_test.hpp"
#include <atomic>
#include <chrono>
#include <iostream>
#include <print>
#include <thread>
#include <vector>
#include "con_stack.hpp"
#include "stack.hpp"

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
            std::println("got here");
            try {
                if (!unsafe_stack.unsafe_empty()) {
                    int value = unsafe_stack.unsafe_top();
                    unsafe_stack.unsafe_pop();
                    consumer_count++;
                }
            } catch (const std::exception& e) {
                std::println("Race condition detected: {}", e.what());
                          ;
                race_detected = true;
            }
            std::this_thread::yield();
        }
    });
    std::this_thread::sleep_for(std::chrono::seconds(2));
    producer.request_stop();
    consumer.request_stop();
    std::println("Producer pushed: {} items", std::to_string(producer_count));
    std::println("Consumer popped: {} items", std::to_string(consumer_count));
    if (race_detected) {
        std::println("A race condition was detected!");
    } else {
        std::println("No race condition detected in this run, but the code is still unsafe.");
        std::println("The absence of a detected race doesn't mean the code is thread-safe.");
    }
}

// Demonstration where mutex is used to ensure safe concurrency
auto StackTest::demonstrate_concurrent_stack() -> void {
    std::println("\n=== Demonstrating Thread-Safe Stack ===");
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
    std::println("Producer pushed: {} items", std::to_string(producer_count));
    std::println("Consumer popped: {} items", std::to_string(consumer_count));
    std::println("Items left in stack: {}", std::to_string(safe_stack.size()));
    std::println("The concurrent implementation handles multiple threads correctly.");
}

// Demonstration where Producer thread occasionally pauses and Consumer thread waits based on cv
auto StackTest::demonstrate_condition_variable() -> void {
    std::println("\n=== Demonstrating Condition Variable Usage ===");
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
    std::println("Producer pushed: {} items", std::to_string(producer_count));
    std::println("Consumer popped: {} items", std::to_string(consumer_count));
    std::println("The condition variable allows efficient waiting without busy-waiting.");
}

auto StackTest::stackTest() -> void {
    StackTest::demonstrate_data_race();
    StackTest::demonstrate_concurrent_stack();
    StackTest::demonstrate_condition_variable();
}