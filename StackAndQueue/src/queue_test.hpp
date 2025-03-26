#pragma once

namespace QueueTest {
    // Demo of data race with the original Queue
    auto demonstrate_data_race() -> void;

    // Demo of thread-safe queue
    auto demonstrate_concurrent_queue() -> void;

    // Demo of condition variable for efficient waiting
    auto demonstrate_condition_variable() -> void;

    auto queueTest() -> void;
}  // namespace QueueTest