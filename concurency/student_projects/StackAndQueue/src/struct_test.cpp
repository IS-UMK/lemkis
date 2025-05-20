#include "include/struct_test.hpp"

#include <atomic>
#include <chrono>
#include <iostream>
#include <print>
#include <thread>
#include <vector>

#include "include/con_queue.hpp"
#include "include/con_stack.hpp"
#include "include/queue.hpp"
#include "include/stack.hpp"

auto struct_test::run_queue_test() -> void {
    const int data_race_items = 10000;
    const int con_queue_items = 10000;
    const int cond_variable_items = 100;
    demonstrate_dr<concurrent_queue<int>>(data_race_items);
    demonstrate_conc<concurrent_queue<int>>("Queue", con_queue_items);
    demonstrate_cv<concurrent_queue<int>>(cond_variable_items);
}

auto struct_test::run_stack_test() -> void {
    const int data_race_items = 10000;
    const int con_stack_items = 10000;
    const int cond_variable_items = 100;
    demonstrate_dr<concurrent_stack<int>>(data_race_items);
    demonstrate_conc<concurrent_stack<int>>("Stack", con_stack_items);
    demonstrate_cv<concurrent_stack<int>>(cond_variable_items);
}

template <typename SName>
auto struct_test::demonstrate_dr(const int item_count) -> void {
    std::println("=== Demonstrating Data Race Issues ===");
    SName unsafe_struct;
    std::atomic<int> producer_count(0);
    std::atomic<int> consumer_count(0);
    std::atomic<bool> race_detected(false);
    std::jthread producer = dr_create_producer(
        unsafe_struct, item_count, producer_count, race_detected);
    std::jthread consumer = dr_create_consumer(
        unsafe_struct, item_count, consumer_count, race_detected);
    wait(producer, consumer);
    dr_print_results(producer_count, consumer_count, race_detected);
}

template <typename SName>
auto struct_test::dr_create_producer(SName& s,
                                     int item_count,
                                     std::atomic<int>& count,
                                     std::atomic<bool>& race_flag)
    -> std::jthread {
    return std::jthread([&](const std::stop_token& stoken) {
        for (int i = 0; dr_should_continue(i, item_count, stoken, race_flag);
             ++i) {
            s.unsafe_push(i);
            count++;
            std::this_thread::yield();
        }
    });
}

template <typename SName>
auto struct_test::dr_create_consumer(SName& s,
                                     int item_count,
                                     std::atomic<int>& count,
                                     std::atomic<bool>& race_flag)
    -> std::jthread {
    return std::jthread([&](const std::stop_token& stoken) {
        for (int i = 0; dr_should_continue(i, item_count, stoken, race_flag);
             ++i) {
            dr_try_consume(s, count, race_flag);
            std::this_thread::yield();
        }
    });
}

auto struct_test::dr_should_continue(int current,
                                     int max,
                                     const std::stop_token& stoken,
                                     const std::atomic<bool>& race_flag)
    -> bool {
    return current < max && !stoken.stop_requested() && !race_flag;
}

template <typename SName>
auto struct_test::dr_try_consume(SName& s,
                                 std::atomic<int>& count,
                                 std::atomic<bool>& race_flag) -> void {
    try {
        if (!s.unsafe_empty()) {
            s.unsafe_pop();
            count++;
        }
    } catch (const std::exception& e) {
        std::println("Race condition detected: {}", e.what());
        race_flag = true;
    }
}

auto struct_test::dr_print_results(const std::atomic<int>& produced,
                                   const std::atomic<int>& consumed,
                                   bool race_detected) -> void {
    std::println("Producer pushed: {} items", produced.load());
    std::println("Consumer popped: {} items", consumed.load());
    if (race_detected) {
        std::println("A race condition was detected!");
        return;
    }
    std::println("No race condition detected in this run, but the code is",
                 "still unsafe. \n The absence of a detected race doesn't",
                 "mean the code is thread-safe.");
}

template <typename SName>
auto struct_test::demonstrate_conc(const std::string& name,
                                   const int item_count) -> void {
    std::println("\n=== Demonstrating Thread-Safe {} ===", name);
    SName safe_struct;
    std::atomic<int> producer_count(0);
    std::atomic<int> consumer_count(0);
    std::jthread producer =
        conc_create_producer(safe_struct, item_count, producer_count);
    std::jthread consumer = conc_create_consumer(safe_struct, consumer_count);
    wait(producer, consumer);
    conc_print_results(producer_count, consumer_count, safe_struct);
}

template <typename SName>
auto struct_test::conc_create_producer(SName& safe_struct,
                                       int item_count,
                                       std::atomic<int>& count)
    -> std::jthread {
    return std::jthread([&](const std::stop_token& stoken) {
        for (int i = 0; i < item_count && !stoken.stop_requested(); ++i) {
            safe_struct.push(i);
            count++;
            std::this_thread::yield();
        }
    });
}

template <typename SName>
auto struct_test::conc_create_consumer(SName& safe_struct,
                                       std::atomic<int>& count)
    -> std::jthread {
    return std::jthread([&](const std::stop_token& stoken) {
        conc_consumer_loop(safe_struct, count, stoken);
    });
}

template <typename SName>
auto struct_test::conc_consumer_loop(SName& safe_struct,
                                     std::atomic<int>& count,
                                     const std::stop_token& stoken) -> void {
    while (!stoken.stop_requested() || !safe_struct.empty()) {
        if (safe_struct.try_pop()) { count++; }
        std::this_thread::yield();
    }
}

template <typename SName>
auto struct_test::conc_print_results(const std::atomic<int>& produced,
                                     const std::atomic<int>& consumed,
                                     const SName& safe_struct) -> void {
    std::println("Producer pushed: {} items", produced.load());
    std::println("Consumer popped: {} items", consumed.load());
    std::println("Items left: {}", safe_struct.size());
    std::println(
        "The concurrent implementation handles multiple threads "
        "correctly.");
}

template <typename SName>
auto struct_test::demonstrate_cv(const int item_count) -> void {
    std::println("\n=== Demonstrating Condition Variable Usage ===");
    SName safe_struct;
    std::atomic<int> producer_count(0);
    std::atomic<int> consumer_count(0);
    auto producer = cv_create_producer(safe_struct, item_count, producer_count);
    auto consumer = cv_create_consumer(safe_struct, consumer_count);
    wait(producer, consumer);
    cv_print_results(producer_count, consumer_count);
}

template <typename SName>
auto struct_test::cv_create_producer(SName& safe_struct,
                                     int item_count,
                                     std::atomic<int>& count) -> std::jthread {
    return std::jthread([&](const std::stop_token& stoken) {
        cv_producer_loop(safe_struct, item_count, count, stoken);
    });
}

template <typename SName>
auto struct_test::cv_producer_loop(SName& safe_struct,
                                   int item_count,
                                   std::atomic<int>& count,
                                   const std::stop_token& stoken) -> void {
    constexpr int sleep_time_ms = 100;
    constexpr int pushes_until_sleep = 10;
    for (int i = 0; i < item_count && !stoken.stop_requested(); ++i) {
        if (i % pushes_until_sleep == 0) {
            std::this_thread::sleep_for(
                std::chrono::milliseconds(sleep_time_ms));
        }
        cv_producer_work(safe_struct, i, count);
    }
}

template <typename SName>
auto struct_test::cv_producer_work(SName& safe_struct,
                                   int i,
                                   std::atomic<int>& count) -> void {
    safe_struct.push(i);
    count++;
}

template <typename SName>
auto struct_test::cv_create_consumer(SName& safe_struct,
                                     std::atomic<int>& count) -> std::jthread {
    return std::jthread([&](const std::stop_token& stoken) {
        while (!stoken.stop_requested() || !safe_struct.empty()) {
            int value;
            safe_struct.try_top(value);
            safe_struct.pop();
            std::println("Consumer got value: {}", value);
            count++;
        }
    });
}

auto struct_test::cv_print_results(const std::atomic<int>& produced,
                                   const std::atomic<int>& consumed) -> void {
    std::println("Producer pushed: {} items", produced.load());
    std::println("Consumer popped: {} items", consumed.load());
    std::println(
        "The condition variable allows efficient waiting without "
        "busy-waiting.");
}

auto struct_test::wait(std::jthread& producer, std::jthread& consumer) -> void {
    const int duration = 2;
    std::this_thread::sleep_for(std::chrono::seconds(duration));
    producer.request_stop();
    consumer.request_stop();
}