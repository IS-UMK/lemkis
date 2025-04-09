#pragma once

#include <atomic>
#include <chrono>
#include <iostream>
#include <print>
#include <thread>
#include <vector>

#include "con_queue.hpp"
#include "queue.hpp"

namespace QueueTest {
    // Demo of data race with the original Queue
    auto demonstrate_data_race(const int item_count) -> void;

    // Demo of thread-safe queue
    auto demonstrate_concurrent_queue(const int item_count) -> void;

    // Demo of condition variable for efficient waiting
    auto demonstrate_condition_variable(const int item_count) -> void;

    auto queueTest() -> void;
}  // namespace QueueTest