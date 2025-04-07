#pragma once

#include <condition_variable>
#include <mutex>
#include <optional>
#include "vector_stack.hpp"

// Queue implementation using two stacks (based on VectorStack)
template <typename T>
class TwoStackQueue {
  private:
    VectorStack<T> m_stack_input;   // For enqueue operations
    VectorStack<T> m_stack_output;  // For dequeue operations
    mutable std::mutex m_mutex;
    std::condition_variable m_cv;

    // Helper method to transfer elements from input stack to output stack
    void transfer_if_needed() {
        // Only transfer if output stack is empty (no need for a lock here as
        // this is called within methods that already hold the lock)
        if (m_stack_output.unsafe_empty()) {
            // Move all elements from input stack to output stack
            while (auto item = m_stack_input.unsafe_pop()) {
                m_stack_output.unsafe_push(*item);
            }
        }
    }

  public:
    // Unsafe methods (not thread-safe)
    void unsafe_enqueue(T value) {
        m_stack_input.unsafe_push(std::move(value));
    }

    std::optional<T> unsafe_dequeue() {
        if (m_stack_output.unsafe_empty()) {
            // If output stack is empty, transfer elements from input stack
            transfer_if_needed();

            // If still empty after transfer, queue is empty
            if (m_stack_output.unsafe_empty()) { return std::nullopt; }
        }

        return m_stack_output.unsafe_pop();
    }

    bool unsafe_empty() const {
        return m_stack_input.unsafe_empty() && m_stack_output.unsafe_empty();
    }

    size_t unsafe_size() const {
        return m_stack_input.unsafe_size() + m_stack_output.unsafe_size();
    }

    // Safe methods using mutex
    void mutex_enqueue(T value) {
        std::lock_guard<std::mutex> lock(m_mutex);
        unsafe_enqueue(std::move(value));
    }

    std::optional<T> mutex_dequeue() {
        std::lock_guard<std::mutex> lock(m_mutex);
        return unsafe_dequeue();
    }

    // Safe methods using condition variable
    void cv_enqueue(T value) {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            unsafe_enqueue(std::move(value));
        }
        m_cv.notify_all();
    }

    // This will wait until there's an item to dequeue
    T cv_dequeue_wait() {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_cv.wait(lock, [this] { return !this->unsafe_empty(); });
        return *unsafe_dequeue();
    }

    // This will return nullopt if empty
    std::optional<T> cv_dequeue() {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (unsafe_empty()) { return std::nullopt; }
        return unsafe_dequeue();
    }

    // Same for cv and mutex versions since they only read
    bool empty() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return unsafe_empty();
    }

    size_t size() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return unsafe_size();
    }
};