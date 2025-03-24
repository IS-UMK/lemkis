#pragma once

#include <condition_variable>
#include <memory>
#include <mutex>

#include "node.hpp"

template <typename T>
class ConcurrentStack {
  private:
    std::unique_ptr<Node<T>> top_;
    std::size_t size_;

    // Synchronization primitives
    mutable std::mutex mutex_;
    std::condition_variable not_empty_cv_;

    // Helper method for internal use when lock is already held
    auto push_unsafe(T value) -> void {
        auto newNode = std::make_unique<Node<T>>(std::move(value));
        newNode->next = std::move(top_);
        top_ = std::move(newNode);

        size_++;
    }

    // Helper method to check if stack is empty when lock is already held
    auto empty_unsafe() const -> bool { return top_ == nullptr; }

  public:
    // Constructor
    ConcurrentStack() = default;

    // Destructor
    ~ConcurrentStack() = default;

    // Copy constructor
    ConcurrentStack(const ConcurrentStack& other) {
        std::lock_guard<std::mutex> lock(other.mutex_);

        top_ = nullptr;
        size_ = other.size_;
        if (other.top_ == nullptr) return;
        top_ = std::make_unique<Node<T>>(std::move(other.top_->data));
        Node<T>* thisCurrent = top_.get();
        Node<T>* otherNext = other.top_->next.get();
        while (otherNext) {
            thisCurrent->next =
                std::make_unique<Node<T>>(std::move(otherNext->data));
            thisCurrent = thisCurrent->next.get();
            otherNext = otherNext->next.get();
        }
    }

    // Move constructor
    ConcurrentStack(ConcurrentStack&& other) noexcept {
        std::lock_guard<std::mutex> lock(other.mutex_);

        top_ = std::move(other.top_);
        size_ = other.size_;
        other.size_ = 0;
    }

    // Copy assignment operator
    auto operator=(const ConcurrentStack& other) -> ConcurrentStack& {
        if (this != &other) {
            // Need to lock both stacks to avoid deadlock
            std::lock(mutex_, other.mutex);
            std::lock_guard<std::mutex> lock_this(mutex_, std::adopt_lock);
            std::lock_guard<std::mutex> lock_other(other.mutex_,
                                                   std::adopt_lock);

            // Clear current stack
            top_.reset();
            size_ = other.size_;

            // Copy elements from other stack
            if (other.top_ == nullptr) return *this;
            top_ = std::make_unique<Node<T>>(std::move(other.top_->data));
            Node<T>* thisCurrent = top_.get();
            Node<T>* otherNext = other.top_->next.get();
            while (otherNext) {
                thisCurrent->next =
                    std::make_unique<Node<T>>(std::move(otherNext->data));
                thisCurrent = thisCurrent->next.get();
                otherNext = otherNext->next.get();
            }
        }

        return *this;
    }

    // Move assignment operator
    auto operator=(ConcurrentStack&& other) noexcept -> ConcurrentStack& {
        if (this != &other) {
            std::lock(mutex_, other.mutex);
            std::lock_guard<std::mutex> lock_this(mutex_, std::adopt_lock);
            std::lock_guard<std::mutex> lock_other(other.mutex_,
                                                   std::adopt_lock);

            top_ = std::move(other.top_);
            size_ = other.size_;

            other.size_ = 0;
        }
        return *this;
    }

    // Add an element to the stack
    auto push(T value) -> void {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            push_unsafe(std::move(value));
        }
        // Notify one waiting thread that data is available
        not_empty_cv_.notify_one();
    }

    // Remove the top element (non-blocking)
    auto try_pop(T& value) -> bool {
        std::lock_guard<std::mutex> lock(mutex_);
        if (empty_unsafe()) { return false; }

        value = std::move(top_->data);
        top_ = std::move(top_->next);
        size_--;
        return true;
    }

    // Remove the top element (blocking)
    auto pop(T& value) -> void {
        std::unique_lock<std::mutex> lock(mutex_);
        not_empty_cv_.wait(lock, [this] { return !empty_unsafe(); });

        value = std::move(top_->data);
        top_ = std::move(top_->next);
        size_--;
    }

    // Timed wait pop
    auto pop(T& value, std::chrono::milliseconds timeout) -> bool {
        std::unique_lock<std::mutex> lock(mutex_);
        bool success = not_empty_cv_.wait_for(
            lock, timeout, [this] { return !empty_unsafe(); });

        if (!success) {
            return false;  // Timed out
        }

        value = std::move(top_->data);
        top_ = std::move(top_->next);
        size_--;
        return true;
    }

    // Try to peek at top element without removing
    auto try_top(T& value) const -> bool {
        std::lock_guard<std::mutex> lock(mutex_);
        if (empty_unsafe()) { return false; }
        value = top_->data;
        return true;
    }

    // Check if the stack is empty
    [[nodiscard]] auto empty() const -> bool {
        std::lock_guard<std::mutex> lock(mutex_);
        return top_ == nullptr;
    }

    // Get the size of the stack
    [[nodiscard]] auto size() const -> std::size_t {
        std::lock_guard<std::mutex> lock(mutex_);
        return size_;
    }
};