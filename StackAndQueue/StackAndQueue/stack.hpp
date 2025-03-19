#pragma once

#include <concepts>
#include <iostream>
#include <memory>
#include <optional>
#include <stdexcept>
#include "node.hpp"

template <typename T>
requires std::is_default_constructible_v<T>
class Stack {
  private:
    std::unique_ptr<Node<T>> topNode;

  public:
    Stack() : topNode(nullptr) {}

    // Push an element onto the stack
    auto push(T value) -> void {
        auto newNode = std::make_unique<Node<T>>(std::move(value));
        newNode->next = std::move(topNode);
        topNode = std::move(newNode);
    }

    // Pop the top element from the stack
    auto pop() -> void {
        if (!topNode) {
            throw std::underflow_error(
                "Stack underflow: Cannot pop from an empty stack.");
        }
        topNode = std::move(topNode->next);
    }

    // Get the top element of the stack
    auto top() const -> std::optional<T> {
        if (!topNode)
            return std::nullopt;
        return topNode->data;
    }

    // Check if the stack is empty
    bool empty() const {
        return topNode == nullptr;
    }
};