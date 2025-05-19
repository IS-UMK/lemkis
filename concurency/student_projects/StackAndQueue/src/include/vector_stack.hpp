#pragma once

#include <condition_variable>
#include <mutex>
#include <optional>
#include <vector>

// Vector-based stack implementation
template <typename T>
class vector_stack {
  private:
    std::vector<T> m_data;
    mutable std::mutex m_mutex;
    std::condition_variable m_cv;

  public:
    // Unsafe methods (not thread-safe)
    void unsafe_push(T value) { m_data.push_back(std::move(value)); }

    auto unsafe_pop() -> std::optional<T> {
        if (m_data.empty()) { return std::nullopt; }

        T value = std::move(m_data.back());
        m_data.pop_back();
        return value;
    }

    auto unsafe_empty() const -> bool { return m_data.empty(); }

    auto unsafe_size() const -> size_t { return m_data.size(); }

    // Safe methods using mutex
    void mutex_push(T value) {
        std::lock_guard<std::mutex> lock(m_mutex);
        unsafe_push(std::move(value));
    }

    auto mutex_pop() -> std::optional<T> {
        std::lock_guard<std::mutex> lock(m_mutex);
        return unsafe_pop();
    }

    // Safe methods using condition variable
    void cv_push(T value) {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            unsafe_push(std::move(value));
        }
        m_cv.notify_all();
    }

    // This will wait until there's an item to pop
    auto cv_pop_wait() -> T {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_cv.wait(lock, [this] { return !this->unsafe_empty(); });
        return *unsafe_pop();
    }

    // This will return nullopt if empty
    auto cv_pop() -> std::optional<T> {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (unsafe_empty()) { return std::nullopt; }
        return unsafe_pop();
    }

    // Same for mutex and cv since they only read
    auto empty() const -> bool {
        std::lock_guard<std::mutex> lock(m_mutex);
        return unsafe_empty();
    }

    auto size() const -> size_t {
        std::lock_guard<std::mutex> lock(m_mutex);
        return unsafe_size();
    }
};