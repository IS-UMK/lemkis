#pragma once

#include <condition_variable>
#include <mutex>
#include <optional>

#include "vector_stack.hpp"

// Queue implementation using two stacks (based on vector_stack)
template <typename T>
class two_stack_queue {
  private:
    vector_stack<T> m_stack_input;   // For enqueue operations
    vector_stack<T> m_stack_output;  // For dequeue operations
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

    auto unsafe_dequeue() -> std::optional<T> {
        if (m_stack_output.unsafe_empty()) {
            // If output stack is empty, transfer elements from input stack
            transfer_if_needed();

            // If still empty after transfer, queue is empty
            if (m_stack_output.unsafe_empty()) { return std::nullopt; }
        }

        return m_stack_output.unsafe_pop();
    }

    auto unsafe_empty() const -> bool {
        return m_stack_input.unsafe_empty() && m_stack_output.unsafe_empty();
    }

    auto unsafe_size() const -> size_t {
        return m_stack_input.unsafe_size() + m_stack_output.unsafe_size();
    }

    // Safe methods using mutex
    auto mutex_enqueue(T value) -> void {
        std::lock_guard<std::mutex> lock(m_mutex);
        unsafe_enqueue(std::move(value));
    }

    auto mutex_dequeue() -> std::optional<T> {
        std::lock_guard<std::mutex> lock(m_mutex);
        return unsafe_dequeue();
    }

    // Safe methods using condition variable
    auto cv_enqueue(T value) -> void {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            unsafe_enqueue(std::move(value));
        }
        m_cv.notify_all();
    }

    // This will wait until there's an item to dequeue
    auto cv_dequeue_wait() -> T {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_cv.wait(lock, [this] { return !this->unsafe_empty(); });
        return *unsafe_dequeue();
    }

    // This will return nullopt if empty
    auto cv_dequeue() -> std::optional<T> {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (unsafe_empty()) { return std::nullopt; }
        return unsafe_dequeue();
    }

    // Same for cv and mutex versions since they only read
    auto empty() const -> bool {
        std::lock_guard<std::mutex> lock(m_mutex);
        return unsafe_empty();
    }

    auto size() const -> size_t {
        std::lock_guard<std::mutex> lock(m_mutex);
        return unsafe_size();
    }
};