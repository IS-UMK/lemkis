#pragma once

#include <condition_variable>
#include <memory>
#include <mutex>

#include "node.hpp"

template <typename T>
class concurrent_queue {
  private:
    std::unique_ptr<node<T>> front_;
    node<T>* rear_ = nullptr;
    std::size_t size_ = 0;

    // Synchronization primitives
    mutable std::mutex mutex_;
    std::condition_variable not_empty_cv_;

  public:
    // Constructor
    concurrent_queue() = default;

    // Destructor
    ~concurrent_queue() = default;

    // Helper method for internal use when lock is already held
    auto unsafe_push(T value) -> void {
        auto new_node = std::make_unique<node<T>>(std::move(value));
        if (front_ == nullptr) {
            front_ = std::move(new_node);
            rear_ = front_.get();
        } else {
            rear_->next = std::move(new_node);
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

    // Access the front element
    auto unsafe_peek() const -> T& { return front_->data; }

    // Try to peek at front element without removing
    auto try_peek(T& value) const -> bool {
        std::lock_guard<std::mutex> lock(mutex_);
        if (unsafe_empty()) { return false; }
        value = unsafe_peek();
        return true;
    }

    // Alias for try_peek
    auto try_top(T& value) const -> bool { return try_peek(value); }

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