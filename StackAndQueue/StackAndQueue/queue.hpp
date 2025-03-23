#pragma once

#include "node.hpp"

#include <memory>

template <typename T>
class Queue {
  private:
    std::unique_ptr<Node<T>> front_;
    Node<T>* rear_ = nullptr;
    std::size_t size_ = 0;

  public:
    // Constructor
    Queue() = default;

    // Destructor
    ~Queue() = default;

    // Copy constructor
    Queue(const Queue& other) {
        Node<T>* current = other.front_.get();
        while (current != nullptr) {
            push(current->data);
            current = current->next.get();
        }
    }

    // Move constructor
    Queue(Queue&& other) noexcept
        : front_(std::move(other.front_)),
          rear_(other.rear_),
          size_(other.size_) {
        other.rear_ = nullptr;
        other.size_ = 0;
    }

    // Copy assignment operator
    auto operator=(const Queue& other) -> Queue& {
        if (this != &other) {
            // Clear current queue
            front_.reset();
            rear_ = nullptr;
            size_ = 0;

            // Copy elements from other queue
            Node<T>* current = other.front_.get();
            while (current != nullptr) {
                push(current->data);
                current = current->next.get();
            }
        }
        return *this;
    }

    // Move assignment operator
    auto operator=(Queue&& other) noexcept -> Queue& {
        if (this != &other) {
            front_ = std::move(other.front_);
            rear_ = other.rear_;
            size_ = other.size_;

            other.rear_ = nullptr;
            other.size_ = 0;
        }
        return *this;
    }

    // Add an element to the queue
    auto push(T value) -> void {
        auto newNode = std::make_unique<Node<T>>(std::move(value));

        if (empty()) {
            front_ = std::move(newNode);
            rear_ = front_.get();
        } else {
            rear_->next = std::move(newNode);
            rear_ = rear_->next.get();
        }

        size_++;
    }

    // Remove the front element
    auto pop() -> void {
        // Precondition: !empty()
        // Calling pop() on an empty queue is undefined behavior

        if (front_.get() == rear_) { rear_ = nullptr; }

        front_ = std::move(front_->next);
        size_--;
    }

    // Access the front element
    auto front() -> T& {
        // Precondition: !empty()
        // Calling front() on an empty queue is undefined behavior
        return front_->data;
    }

    // Const version of front
    auto front() const -> const T& {
        // Precondition: !empty()
        // Calling front() on an empty queue is undefined behavior
        return front_->data;
    }

    // Access the back element
    auto back() -> T& {
        // Precondition: !empty()
        // Calling back() on an empty queue is undefined behavior
        return rear_->data;
    }

    // Const version of back
    auto back() const -> const T& {
        // Precondition: !empty()
        // Calling back() on an empty queue is undefined behavior
        return rear_->data;
    }

    // Check if the queue is empty
    [[nodiscard]] auto empty() const -> bool { return front_ == nullptr; }

    // Get the size of the queue
    [[nodiscard]] auto size() const -> std::size_t { return size_; }
};