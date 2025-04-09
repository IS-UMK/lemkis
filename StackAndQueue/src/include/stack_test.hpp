#pragma once

#include <atomic>
#include <chrono>
#include <iostream>
#include <print>
#include <thread>
#include <vector>

#include "con_stack.hpp"
#include "stack.hpp"

namespace StackTest {
    // Demo of data race with the original Stack
    auto demonstrate_data_race(const int item_count) -> void;

    // Demo of thread-safe stack
    auto demonstrate_concurrent_stack(const int item_count) -> void;

    // Demo of condition variable for efficient waiting
    auto demonstrate_condition_variable(const int item_count) -> void;

    auto stackTest() -> void;
}