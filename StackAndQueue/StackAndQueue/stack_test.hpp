#pragma once

class StackTest {
public:
    // Demo of data race with the original Stack
    static auto demonstrate_data_race() -> void;

    // Demo of thread-safe stack
    static auto demonstrate_concurrent_stack() -> void;

    // Demo of condition variable for efficient waiting
    static auto demonstrate_condition_variable() -> void;

    static auto stackTest() -> void;
};