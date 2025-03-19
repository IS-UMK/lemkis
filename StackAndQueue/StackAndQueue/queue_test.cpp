#include "queue.hpp"
#include <print>
#include "queue_test.hpp"

auto queueTest() -> void {
    Queue<int> queue;

    // Enqueue some elements
    queue.enqueue(10);
    queue.enqueue(20);
    queue.enqueue(30);

    // Print the queue size
    std::println("Queue size: {}", queue.size());

    // Peek at the front element
    std::println("Front element: {}", queue.peek());

    // Dequeue and print elements
    while (!queue.empty()) { std::println("Dequeued: {}", queue.dequeue()); }

    // Try to use auto for type inference
    auto stringQueue = Queue<std::string>();
    stringQueue.enqueue("Hello");
    stringQueue.enqueue("World");

    std::println("String queue size: {}", stringQueue.size());
}