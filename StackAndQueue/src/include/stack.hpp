#pragma once

#include <memory>

#include "node.hpp"

template <typename T>
class Stack {
  private:
    std::unique_ptr<Node<T>> top_;
    std::size_t size_;

  public:
    Stack() = default;

    ~Stack() = default;

    Stack(const Stack& other) : top_(nullptr), size_(other.size_) {
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
    Stack(Stack&& other) noexcept
        : top_(std::move(other.top_)), size_(other.size_) {
        other.size_ = 0;
    }
    auto operator=(const Stack& other) -> Stack& {
        if (this != &other) {
            top_.reset();
            size_ = other.size_;
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
    auto operator=(Stack&& other) noexcept -> Stack& {
        if (this != &other) {
            top_ = std::move(other.top_);
            size_ = other.size_;

            other.size_ = 0;
        }
        return *this;
    }
    auto push(T value) -> void {
        auto newNode = std::make_unique<Node<T>>(std::move(value));
        newNode->next = std::move(top_);
        top_ = std::move(newNode);

        size_++;
    }
    auto pop() -> void {

        top_ = std::move(top_->next);
        size_--;
    }
    auto top() -> T& { return top_->data; }
    auto top() const -> const T& { return top_->data; }
    [[nodiscard]] auto empty() const -> bool { return top_ == nullptr; }
    [[nodiscard]] auto size() const -> std::size_t { return size_; }
};