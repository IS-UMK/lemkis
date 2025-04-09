#pragma once

#include <condition_variable>
#include <memory>
#include <mutex>

#include "node.hpp"

template <typename T>
class ConcurrentQueue {
  private:
    std::unique_ptr<Node<T>> front_;
    Node<T>* rear_ = nullptr;
    std::size_t size_ = 0;

    // Synchronization primitives
    mutable std::mutex mutex_;
    std::condition_variable not_empty_cv_;

  public:
    // Constructor
    ConcurrentQueue() = default;

    // Destructor
    ~ConcurrentQueue() = default;

    // Copy constructor
    ConcurrentQueue(const ConcurrentQueue& other) {
        std::lock_guard<std::mutex> lock(other.mutex_);
        Node<T>* current = other.front_.get();
        while (current != nullptr) {
            push(current->data);
            current = current->next.get();
        }
    }

    // Move constructor
    ConcurrentQueue(ConcurrentQueue&& other) noexcept {
        std::lock_guard<std::mutex> lock(other.mutex_);
        front_ = std::move(other.front_);
        rear_ = other.rear_;
        size_ = other.size_;
        other.rear_ = nullptr;
        other.size_ = 0;
    }

    // Copy assignment operator
    auto operator=(const ConcurrentQueue& other) -> ConcurrentQueue& {
        if (this != &other) {
            // Need to lock both queues to avoid deadlock
            std::lock(mutex_, other.mutex_);
            std::lock_guard<std::mutex> lock_this(mutex_, std::adopt_lock);
            std::lock_guard<std::mutex> lock_other(other.mutex_,
                                                   std::adopt_lock);

            // Clear current queue
            front_.reset();
            rear_ = nullptr;
            size_ = 0;

            // Copy elements from other queue
            Node<T>* current = other.front_.get();
            while (current != nullptr) {
                push_unsafe(current->data);  // Using unsafe version since we
                // already have the lock
                current = current->next.get();
            }
        }
        return *this;
    }

    // Move assignment operator
    auto operator=(ConcurrentQueue&& other) noexcept -> ConcurrentQueue& {
        if (this != &other) {
            std::lock(mutex_, other.mutex_);
            std::lock_guard<std::mutex> lock_this(mutex_, std::adopt_lock);
            std::lock_guard<std::mutex> lock_other(other.mutex_,
                                                   std::adopt_lock);
            front_ = std::move(other.front_);
            rear_ = other.rear_;
            size_ = other.size_;
            other.rear_ = nullptr;
            other.size_ = 0;
        }
        return *this;
    }

    // Helper method for internal use when lock is already held
    auto unsafe_push(T value) -> void {
        auto newNode = std::make_unique<Node<T>>(std::move(value));
        if (front_ == nullptr) {
            front_ = std::move(newNode);
            rear_ = front_.get();
        } else {
            rear_->next = std::move(newNode);
            rear_ = rear_->next.get();
        }
        size_++;
    }

    // Add an element to the queue
    auto push(T value) -> void {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            unsafe_push(std::move(value));
        }
        // Notify one waiting thread that data is available
        not_empty_cv_.notify_one();
    }

    // Remove the front element (non-blocking)
    auto try_pop() -> bool {
        std::lock_guard<std::mutex> lock(mutex_);
        if (unsafe_empty()) { return false; }
        unsafe_pop();
        return true;
    }

    // Remove the front element
    auto unsafe_pop() -> void {
        // Precondition: !empty()
        // Calling pop() on an empty queue is undefined behavior
        if (front_.get() == rear_) { rear_ = nullptr; }
        front_ = std::move(front_->next);
        size_--;
    }

    // Pop with use of cv
    auto pop() -> void {
        std::unique_lock<std::mutex> lock(mutex_);
        not_empty_cv_.wait(lock, [this] { return !unsafe_empty(); });
        unsafe_pop();
    }

    // Try to peek at front element without removing
    auto try_peek(T& value) const -> bool {
        std::lock_guard<std::mutex> lock(mutex_);
        if (unsafe_empty()) { return false; }
        value = front_->data;
        return true;
    }

    // Helper method to check if queue is empty when lock is already held
    auto unsafe_empty() const -> bool { return front_ == nullptr; }

    // Check if the queue is empty
    [[nodiscard]] auto empty() const -> bool {
        std::lock_guard<std::mutex> lock(mutex_);
        return unsafe_empty();
    }

    // Get the size of the queue
    [[nodiscard]] auto size() const -> std::size_t {
        std::lock_guard<std::mutex> lock(mutex_);
        return size_;
    }
};