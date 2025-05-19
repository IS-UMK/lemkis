#pragma once

#include <atomic>
#include <chrono>
#include <iostream>
#include <print>
#include <thread>
#include <vector>

#include "con_queue.hpp"
#include "queue.hpp"

namespace queue_test {
    // Demo of data race with the original queue
    auto demonstrate_data_race(int item_count) -> void;

    // Demo of thread-safe queue
    auto demonstrate_concurrent_queue(int item_count) -> void;

    // Demo of condition variable for efficient waiting
    auto demonstrate_condition_variable(int item_count) -> void;

    auto run_queue_test() -> void;
}  // namespace queue_test