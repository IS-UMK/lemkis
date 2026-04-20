#pragma once

#include <condition_variable>
#include <memory>
#include <mutex>

#include "node.hpp"

// Thread-safe queue implementation using singly-linked list and synchronization
// primitives. Provides both blocking and non-blocking operations for push/pop
// access.
template <typename T>
class concurrent_queue {
  private:
    // Pointer to the front (head) of the queue
    std::unique_ptr<node<T>> front_;
    // Raw pointer to the rear (tail) of the queue
    node<T>* rear_ = nullptr;
    // Current number of elements in the queue
    std::size_t size_ = 0;

    // Mutex to protect access to the queue
    mutable std::mutex mutex_;
    // Condition variable to signal availability of data
    std::condition_variable not_empty_cv_;

  public:
    // Constructs an empty queue
    concurrent_queue() = default;

    // Default destructor (unique_ptr will clean up nodes automatically)
    ~concurrent_queue() = default;

    // Internal helper method to push without locking.
    // Assumes caller already holds the mutex.
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

    // Adds an element to the queue (thread-safe)
    auto push(T value) -> void {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            unsafe_push(std::move(value));
        }
        // Notify one thread waiting on pop()
        not_empty_cv_.notify_one();
    }

    // Attempts to remove the front element without blocking.
    // Returns true if an element was removed, false if the queue was empty.
    auto try_pop() -> bool {
        std::lock_guard<std::mutex> lock(mutex_);
        if (unsafe_empty()) { return false; }
        unsafe_pop();
        return true;
    }

    // Internal method to remove the front element without locking.
    // Assumes the queue is not empty and mutex is held.
    auto unsafe_pop() -> void {
        // If there's only one element, reset rear pointer
        if (front_.get() == rear_) { rear_ = nullptr; }
        front_ = std::move(front_->next);
        size_--;
    }

    // Removes the front element, blocking if the queue is empty.
    auto pop() -> void {
        std::unique_lock<std::mutex> lock(mutex_);
        not_empty_cv_.wait(lock, [this] { return !unsafe_empty(); });
        unsafe_pop();
    }

    // Attempts to peek at the front element without removing it.
    // Returns true if successful; false if queue is empty.
    auto try_peek(T& value) const -> bool {
        std::lock_guard<std::mutex> lock(mutex_);
        if (unsafe_empty()) { return false; }
        value = front_->data;
        return true;
    }

    // Returns true if the queue is empty (must be called with lock held)
    auto unsafe_empty() const -> bool { return front_ == nullptr; }

    // Thread-safe check whether the queue is empty
    [[nodiscard]] auto empty() const -> bool {
        std::lock_guard<std::mutex> lock(mutex_);
        return unsafe_empty();
    }

    // Thread-safe getter for the queue size
    [[nodiscard]] auto size() const -> std::size_t {
        std::lock_guard<std::mutex> lock(mutex_);
        return size_;
    }
};
