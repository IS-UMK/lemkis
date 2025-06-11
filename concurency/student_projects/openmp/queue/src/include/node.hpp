#pragma once

#include <memory>

// Node structure for a singly-linked list used in concurrent_queue.
// Holds a data element and a unique pointer to the next node.
template <class T>
struct node {
    T data;                      // Stored data
    std::unique_ptr<node> next;  // Pointer to the next node in the list

    // Constructs a node with given data, initializing 'next' to nullptr
    explicit node(T value) : data(std::move(value)), next(nullptr) {}
};
