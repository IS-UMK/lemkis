#include "include/stack_test.hpp"

// Demonstration in which use of unsafe methods leads to data race
auto stack_test::demonstrate_data_race(const int item_count) -> void {
    std::println("=== Demonstrating Data Race Issues ===");
    concurrent_stack<int> unsafe_stack;
    std::atomic<int> producer_count(0);
    std::atomic<int> consumer_count(0);
    std::atomic<bool> race_detected(false);
    std::jthread producer([&](const std::stop_token &stoken) {
        for (int i = 0;
             i < item_count && !stoken.stop_requested() && !race_detected;
             ++i) {
            unsafe_stack.unsafe_push(i);
            producer_count++;
            std::this_thread::yield();
        }
    });
    std::jthread consumer([&](const std::stop_token &stoken) {
        for (int i = 0;
             i < item_count && !stoken.stop_requested() && !race_detected;
             ++i) {
            // std::println("got here");
            try {
                if (!unsafe_stack.unsafe_empty()) {
                    unsafe_stack.unsafe_pop();
                    consumer_count++;
                }
            } catch (const std::exception &e) {
                std::println("Race condition detected: {}", e.what());
                race_detected = true;
            }
            std::this_thread::yield();
        }
    });
    std::this_thread::sleep_for(std::chrono::seconds(2));
    producer.request_stop();
    consumer.request_stop();
    std::println("Producer pushed: {} items", producer_count.load());
    std::println("Consumer popped: {} items", consumer_count.load());
    if (race_detected) {
        std::println("A race condition was detected!");
    } else {
        std::println(
            "No race condition detected in this run, but the code is "
            "still unsafe.");
        std::println(
            "The absence of a detected race doesn't mean the code is "
            "thread-safe.");
    }
}

// Demonstration where mutex is used to ensure safe concurrency
auto stack_test::demonstrate_concurrent_stack(const int item_count) -> void {
    std::println("\n=== Demonstrating Thread-Safe Stack ===");
    concurrent_stack<int> safe_stack;
    std::atomic<int> producer_count(0);
    std::atomic<int> consumer_count(0);
    std::jthread producer([&](const std::stop_token &stoken) {
        for (int i = 0; i < item_count && !stoken.stop_requested(); ++i) {
            safe_stack.push(i);
            producer_count++;
            std::this_thread::yield();
        }
    });
    std::jthread consumer([&](const std::stop_token &stoken) {
        while (!stoken.stop_requested() || !safe_stack.empty()) {
            int value;
            if (safe_stack.try_pop(value)) { consumer_count++; }
            std::this_thread::yield();
        }
    });
    std::this_thread::sleep_for(std::chrono::seconds(2));
    producer.request_stop();
    consumer.request_stop();
    std::println("Producer pushed: {} items", producer_count.load());
    std::println("Consumer popped: {} items", consumer_count.load());
    // std::println("Items left in stack: {}", safe_stack.size());
    std::println(
        "The concurrent implementation handles multiple threads correctly.");
}

// Demonstration where Producer thread occasionally pauses and Consumer thread
// waits based on cv
auto stack_test::demonstrate_condition_variable(const int item_count) -> void {
    const int sleep_time_ms = 100;
    const int pushes_until_sleep = 10;

    std::println("\n=== Demonstrating Condition Variable Usage ===");
    concurrent_stack<int> safe_stack;
    std::atomic<int> producer_count(0);
    std::atomic<int> consumer_count(0);
    std::jthread producer([&](const std::stop_token &stoken) {
        for (int i = 0; i < item_count && !stoken.stop_requested(); ++i) {
            if (i % pushes_until_sleep == 0) {
                std::this_thread::sleep_for(
                    std::chrono::milliseconds(sleep_time_ms));
            }
            safe_stack.push(i);
            producer_count++;
        }
    });
    std::jthread consumer([&](const std::stop_token &stoken) {
        while (!stoken.stop_requested() || !safe_stack.empty()) {
            safe_stack.pop();
            consumer_count++;
        }
    });
    std::this_thread::sleep_for(std::chrono::seconds(1));
    producer.request_stop();
    consumer.request_stop();
    std::println("Producer pushed: {} items", producer_count.load());
    std::println("Consumer popped: {} items", consumer_count.load());
    std::println(
        "The condition variable allows efficient waiting without "
        "busy-waiting.");
}

auto stack_test::run_stack_test() -> void {
    const int data_race_items = 10000;
    const int con_queue_items = 10000;
    const int cond_variable_items = 100;
    stack_test::demonstrate_data_race(data_race_items);
    stack_test::demonstrate_concurrent_stack(con_queue_items);
    stack_test::demonstrate_condition_variable(cond_variable_items);
}