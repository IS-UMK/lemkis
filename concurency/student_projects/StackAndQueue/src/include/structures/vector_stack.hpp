/**
 * @file vector_stack.hpp
 * @brief Thread-safe and non-thread-safe stack implementation based on
 * std::vector.
 */

#pragma once

#include <condition_variable>
#include <mutex>
#include <optional>
#include <vector>

/**
 * @class vector_stack
 * @brief Stack container implemented with std::vector and optional
 * synchronization.
 *
 * Provides unsafe methods for single-threaded use and thread-safe methods using
 * mutexes or condition variables for synchronization.
 *
 * @tparam T Type of elements stored in the stack.
 */
template <typename T>
class vector_stack {
  private:
    std::vector<T> m_data;
    mutable std::mutex m_mutex;
    std::condition_variable m_cv;

  public:
    /**
     * @brief Pushes a value onto the stack (not thread-safe).
     * @param value Value to push.
     */
    auto unsafe_push(T value) -> void { m_data.push_back(std::move(value)); }

    /**
     * @brief Pops a value from the stack (not thread-safe).
     * @return An optional containing the value, or std::nullopt if empty.
     */
    auto unsafe_pop() -> std::optional<T> {
        if (m_data.empty()) { return std::nullopt; }

        T value = std::move(m_data.back());
        m_data.pop_back();
        return value;
    }

    /**
     * @brief Checks whether the stack is empty (not thread-safe).
     * @return true if empty.
     */
    auto unsafe_empty() const -> bool { return m_data.empty(); }

    /**
     * @brief Returns the size of the stack (not thread-safe).
     * @return Number of elements.
     */
    auto unsafe_size() const -> size_t { return m_data.size(); }

    /**
     * @brief Thread-safe push using mutex.
     * @param value Value to push.
     */
    auto mutex_push(T value) -> void {
        std::lock_guard<std::mutex> lock(m_mutex);
        unsafe_push(std::move(value));
    }

    /**
     * @brief Thread-safe pop using mutex.
     * @return An optional containing the value, or std::nullopt if empty.
     */
    auto mutex_pop() -> std::optional<T> {
        std::lock_guard<std::mutex> lock(m_mutex);
        return unsafe_pop();
    }

    /**
     * @brief Thread-safe push using condition variable.
     * @param value Value to push.
     */
    auto cv_push(T value) -> void {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            unsafe_push(std::move(value));
        }
        m_cv.notify_all();
    }

    /**
     * @brief Waits until an element is available and pops it (thread-safe).
     * @return The popped value.
     */
    auto cv_pop_wait() -> T {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_cv.wait(lock, [this] { return !this->unsafe_empty(); });
        return *unsafe_pop();
    }

    /**
     * @brief Non-blocking pop (thread-safe).
     * @return An optional containing the value, or std::nullopt if empty.
     */
    auto cv_pop() -> std::optional<T> {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (unsafe_empty()) { return std::nullopt; }
        return unsafe_pop();
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
