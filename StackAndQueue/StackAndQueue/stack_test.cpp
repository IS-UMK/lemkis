#include "stack_test.hpp"
#include <print>
#include "stack.hpp"

auto stackTest() -> void {
    Stack<int> stack;

    stack.push(10);
    stack.push(20);
    stack.push(30);

    while (!stack.empty()) {
        if (auto topValue = stack.top()) {
            std::println("Top element: {}", *topValue);
        }
        stack.pop();
    }

    // Attempting to pop from an empty stack
    try {
        stack.pop();
    } catch (const std::underflow_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}