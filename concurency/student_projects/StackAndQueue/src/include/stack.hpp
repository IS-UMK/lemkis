#pragma once

#include <memory>

#include "node.hpp"

template <typename T>
class stack {
  private:
    std::unique_ptr<node<T>> top_;
    std::size_t size_;

  public:
    stack() = default;

    ~stack() = default;

    auto push(T value) -> void {
        auto new_node = std::make_unique<node<T>>(std::move(value));
        new_node->next = std::move(top_);
        top_ = std::move(new_node);
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