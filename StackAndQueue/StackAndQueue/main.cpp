#include "stack_test.hpp"
#include "queue_test.hpp"

auto main() -> int {
    QueueTest::queueTest();
    StackTest::stackTest();
    return 0;
}