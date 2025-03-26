#pragma once

namespace StackTest {
    // Demo of data race with the original Stack
    auto demonstrate_data_race() -> void;

    // Demo of thread-safe stack
    auto demonstrate_concurrent_stack() -> void;

    // Demo of condition variable for efficient waiting
    auto demonstrate_condition_variable() -> void;

    auto stackTest() -> void;
}