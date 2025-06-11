/**
 * @file two_stack_queue.hpp
 * @brief Queue implementation using two stacks and optional synchronization.
 */

#pragma once

#include <condition_variable>
#include <mutex>
#include <optional>
#include <vector_stack.hpp>

/**
 * @class two_stack_queue
 * @brief Queue implemented using two internal stacks.
 *
 * This structure simulates a FIFO queue using two LIFO stacks,
 * and provides both thread-unsafe and thread-safe (mutex/condition variable)
 * operations for concurrent access.
 *
 * @tparam T Type of elements stored in the queue.
 */
template <typename T>
class two_stack_queue {
  private:
    vector_stack<T> m_stack_input;
    vector_stack<T> m_stack_output;
    mutable std::mutex m_mutex;
    std::condition_variable m_cv;

    /**
     * @brief Transfers elements from input stack to output stack if needed.
     */
    void transfer_if_needed() {
        if (m_stack_output.unsafe_empty()) {
            while (auto item = m_stack_input.unsafe_pop()) {
                m_stack_output.unsafe_push(*item);
            }
        }
    }

  public:
    /**
     * @brief Pushes a value into the queue (not thread-safe).
     * @param value Value to enqueue.
     */
    auto unsafe_enqueue(T value) -> void {
        m_stack_input.unsafe_push(std::move(value));
    }

    /**
     * @brief Pops a value from the queue (not thread-safe).
     * @return An optional containing the value, or std::nullopt if empty.
     */
    auto unsafe_dequeue() -> std::optional<T> {
        if (m_stack_output.unsafe_empty()) {
            transfer_if_needed();
            if (m_stack_output.unsafe_empty()) { return std::nullopt; }
        }
        return m_stack_output.unsafe_pop();
    }

    /**
     * @brief Checks whether the queue is empty (not thread-safe).
     * @return true if empty.
     */
    auto unsafe_empty() const -> bool {
        return m_stack_input.unsafe_empty() && m_stack_output.unsafe_empty();
    }

    /**
     * @brief Returns the size of the queue (not thread-safe).
     * @return Number of elements.
     */
    auto unsafe_size() const -> size_t {
        return m_stack_input.unsafe_size() + m_stack_output.unsafe_size();
    }

    /**
     * @brief Thread-safe enqueue using a mutex.
     * @param value Value to enqueue.
     */
    auto mutex_enqueue(T value) -> void {
        std::lock_guard<std::mutex> lock(m_mutex);
        unsafe_enqueue(std::move(value));
    }

    /**
     * @brief Thread-safe dequeue using a mutex.
     * @return An optional containing the value, or std::nullopt if empty.
     */
    auto mutex_dequeue() -> std::optional<T> {
        std::lock_guard<std::mutex> lock(m_mutex);
        return unsafe_dequeue();
    }

    /**
     * @brief Thread-safe enqueue using a condition variable.
     * @param value Value to enqueue.
     */
    auto cv_enqueue(T value) -> void {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            unsafe_enqueue(std::move(value));
        }
        m_cv.notify_all();
    }

    /**
     * @brief Waits until an item is available and dequeues it (thread-safe).
     * @return The dequeued value.
     */
    auto cv_dequeue_wait() -> T {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_cv.wait(lock, [this] { return !this->unsafe_empty(); });
        return *unsafe_dequeue();
    }

    /**
     * @brief Non-blocking dequeue (thread-safe).
     * @return An optional containing the value, or std::nullopt if empty.
     */
    auto cv_dequeue() -> std::optional<T> {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (unsafe_empty()) { return std::nullopt; }
        return unsafe_dequeue();
    }

    /**
     * @brief Thread-safe check for emptiness.
     * @return true if empty.
     */
    auto empty() const -> bool {
        std::lock_guard<std::mutex> lock(m_mutex);
        return unsafe_empty();
    }

    /**
     * @brief Thread-safe size query.
     * @return Number of elements.
     */
    auto size() const -> size_t {
        std::lock_guard<std::mutex> lock(m_mutex);
        return unsafe_size();
    }
};
