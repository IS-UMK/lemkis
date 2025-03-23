#pragma once

class QueueTest {
public:
    // Demo of data race with the original Queue
    static auto demonstrate_data_race() -> void;

    // Demo of thread-safe queue
    static auto demonstrate_concurrent_queue() -> void;

    // Demo of condition variable for efficient waiting
    static auto demonstrate_condition_variable() -> void;

    static auto queueTest() -> void;
};