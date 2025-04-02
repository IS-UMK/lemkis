#pragma once

#include "queue_test.hpp"

#include <atomic>
#include <chrono>
#include <iostream>
#include <print>
#include <thread>
#include <vector>

#include "con_queue.hpp"
#include "queue.hpp"

// Demonstration in which use of unsafe methods leads to data race
auto QueueTest::demonstrate_data_race() -> void {
    std::println("=== Demonstrating Data Race Issues ===");
    ConcurrentQueue<int> unsafe_queue;
    std::atomic<bool> stop_flag(false);
    std::atomic<int> producer_count(0);
    std::atomic<int> consumer_count(0);
    std::atomic<bool> race_detected(false);
    std::jthread producer([&](std::stop_token stoken) {
        for (int i = 0; i < 10000 && !stoken.stop_requested() && !race_detected;
             ++i) {
            unsafe_queue.unsafe_push(i);
            producer_count++;
            std::this_thread::yield();
        }
    });
    std::jthread consumer([&](std::stop_token stoken) {
        for (int i = 0; i < 10000 && !stoken.stop_requested() && !race_detected;
             ++i) {
            std::println("got here");
            try {
                if (!unsafe_queue.empty_unsafe()) {
                    unsafe_queue.unsafe_pop();
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
auto QueueTest::demonstrate_concurrent_queue() -> void {
    std::cout << "\n=== Demonstrating Thread-Safe Queue ===" << std::endl;
    ConcurrentQueue<int> safe_queue;
    std::atomic<bool> stop_flag(false);
    std::atomic<int> producer_count(0);
    std::atomic<int> consumer_count(0);
    std::jthread producer([&](std::stop_token stoken) {
        for (int i = 0; i < 10000 && !stoken.stop_requested(); ++i) {
            safe_queue.push(i);
            producer_count++;
            std::this_thread::yield();
        }
    });
    std::jthread consumer([&](std::stop_token stoken) {
        while (!stoken.stop_requested() || !safe_queue.empty()) {
            if (safe_queue.try_pop()) { consumer_count++; }
            std::this_thread::yield();
        }
    });
    std::this_thread::sleep_for(std::chrono::seconds(2));
    producer.request_stop();
    consumer.request_stop();
    std::cout << "Producer pushed: " << producer_count << " items" << std::endl;
    std::cout << "Consumer popped: " << consumer_count << " items" << std::endl;
    std::cout << "Items left in queue: " << safe_queue.size() << std::endl;
    std::cout
        << "The concurrent implementation handles multiple threads correctly."
        << std::endl;
}

// Demonstration where Producer thread occasionally pauses and Consumer thread
// waits based on cv
auto QueueTest::demonstrate_condition_variable() -> void {
    std::cout << "\n=== Demonstrating Condition Variable Usage ==="
              << std::endl;
    ConcurrentQueue<int> safe_queue;
    std::atomic<bool> stop_flag(false);
    std::atomic<int> producer_count(0);
    std::atomic<int> consumer_count(0);
    std::jthread producer([&](std::stop_token stoken) {
        for (int i = 0; i < 100 && !stoken.stop_requested(); ++i) {
            if (i % 10 == 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            safe_queue.push(i);
            producer_count++;
        }
    });
    std::jthread consumer([&](std::stop_token stoken) {
        while (!stoken.stop_requested() || !safe_queue.empty()) {
            int value = 0;
            safe_queue.pop();
            consumer_count++;
            std::cout << "Consumer got value: " << value << std::endl;
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

auto QueueTest::queueTest() -> void {
    QueueTest::demonstrate_data_race();
    QueueTest::demonstrate_concurrent_queue();
    QueueTest::demonstrate_condition_variable();
}