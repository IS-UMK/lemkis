#pragma once

#include <memory>

#include "node.hpp"

template <typename T>
class Stack {
  private:
    std::unique_ptr<Node<T>> top_;
    std::size_t size_;

  public:
    // Constructor
    Stack() = default;

    // Destructor
    ~Stack() = default;

    // Copy constructor
    Stack(const Stack& other) : top_(nullptr), size_(other.size_) {
        if (other.top_ == nullptr) return;
        top_ = std::make_unique<Node<T>>(std::move(other.top_->data));
        Node<T>* thisCurrent = top_.get();
        Node<T>* otherNext = other.top_->next.get();
        while (otherNext) {
            thisCurrent->next =
                std::make_unique<Node<T>>(std::move(otherNext->data));
            thisCurrent = thisCurrent->next.get();
            otherNext = otherNext->next.get();
        }
    }

    // Move constructor
    Stack(Stack&& other) noexcept
        : top_(std::move(other.top_)), size_(other.size_) {
        other.size_ = 0;
    }

    // Copy assignment operator
    auto operator=(const Stack& other) -> Stack& {
        if (this != &other) {
            // Clear current stack
            top_.reset();
            size_ = other.size_;

            // Copy elements from other stack
            if (other.top_ == nullptr) return *this;
            top_ = std::make_unique<Node<T>>(std::move(other.top_->data));
            Node<T>* thisCurrent = top_.get();
            Node<T>* otherNext = other.top_->next.get();
            while (otherNext) {
                thisCurrent->next =
                    std::make_unique<Node<T>>(std::move(otherNext->data));
                thisCurrent = thisCurrent->next.get();
                otherNext = otherNext->next.get();
            }
        }

        return *this;
    }

    // Move assignment operator
    auto operator=(Stack&& other) noexcept -> Stack& {
        if (this != &other) {
            top_ = std::move(other.top_);
            size_ = other.size_;

            other.size_ = 0;
        }
        return *this;
    }

    // Add an element to the stack
    auto push(T value) -> void {
        auto newNode = std::make_unique<Node<T>>(std::move(value));
        newNode->next = std::move(top_);
        top_ = std::move(newNode);

        size_++;
    }

    // Remove the top element
    auto pop() -> void {
        // Precondition: !empty()
        // Calling pop() on an empty stack is undefined behavior

        top_ = std::move(top_->next);
        size_--;
    }

    // Access the top element
    auto top() -> T& {
        // Precondition: !empty()
        // Calling top() on an empty stack is undefined behavior
        return top_->data;
    }

    // Const version of top
    auto top() const -> const T& {
        // Precondition: !empty()
        // Calling top() on an empty stack is undefined behavior
        return top_->data;
    }

    // Check if the stack is empty
    [[nodiscard]] auto empty() const -> bool { return top_ == nullptr; }

    // Get the size of the stack
    [[nodiscard]] auto size() const -> std::size_t { return size_; }
};