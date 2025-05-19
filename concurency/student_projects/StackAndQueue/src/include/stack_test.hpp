#pragma once

#include <atomic>
#include <chrono>
#include <iostream>
#include <print>
#include <thread>
#include <vector>

#include "con_stack.hpp"
#include "stack.hpp"

namespace stack_test {
    // Demo of data race with the original stack
    auto demonstrate_data_race(int item_count) -> void;

    // Demo of thread-safe stack
    auto demonstrate_concurrent_stack(int item_count) -> void;

    // Demo of condition variable for efficient waiting
    auto demonstrate_condition_variable(int item_count) -> void;

    auto run_stack_test() -> void;
}  // namespace stack_test