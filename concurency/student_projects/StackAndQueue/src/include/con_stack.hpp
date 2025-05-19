#pragma once

#include <condition_variable>
#include <memory>
#include <mutex>

#include "node.hpp"

template <typename T>
class concurrent_stack {
  private:
    std::unique_ptr<node<T>> top_;
    std::size_t size_;

    // Synchronization primitives
    mutable std::mutex mutex_;
    std::condition_variable not_empty_cv_;

  public:
    // Constructor
    concurrent_stack() = default;

    // Destructor
    ~concurrent_stack() = default;

    // Helper method for internal use when lock is already held
    auto unsafe_push(T value) -> void {
        auto new_node = std::make_unique<node<T>>(std::move(value));
        new_node->next = std::move(top_);
        top_ = std::move(new_node);
        size_++;
    }

    // Add an element to the stack
    auto push(T value) -> void {
        const std::lock_guard<std::mutex> lock(mutex_);
        unsafe_push(std::move(value));
        not_empty_cv_.notify_one();
    }

    // Remove the top element (non-blocking)
    auto try_pop(T& value) -> bool {
        const std::lock_guard<std::mutex> lock(mutex_);
        if (unsafe_empty()) { return false; }

        value = std::move(top_->data);
        top_ = std::move(top_->next);
        size_--;
        return true;
    }

    // Remove the top element
    auto unsafe_pop() -> void {
        top_ = std::move(top_->next);
        size_--;
    }

    // Remove the top element (blocking)
    auto pop() -> void {
        std::unique_lock<std::mutex> lock(mutex_);
        not_empty_cv_.wait(lock, [this] { return !unsafe_empty(); });
        unsafe_pop();
    }

    // Access the top element
    auto unsafe_top() -> T& { return top_->data; }

    // Try to peek at top element without removing
    auto try_top(T& value) const -> bool {
        std::lock_guard<std::mutex> lock(mutex_);
        if (unsafe_empty()) { return false; }
        value = unsafe_top();
        return true;
    }

    // Helper method to check if stack is empty when lock is already held
    auto unsafe_empty() const -> bool { return top_ == nullptr; }

    // Check if the stack is empty
    [[nodiscard]] auto empty() const -> bool {
        const std::lock_guard<std::mutex> lock(mutex_);
        return top_ == nullptr;
    }

    // Get the size of the stack
    [[nodiscard]] auto size() const -> std::size_t {
        const std::lock_guard<std::mutex> lock(mutex_);
        return size_;
    }
};