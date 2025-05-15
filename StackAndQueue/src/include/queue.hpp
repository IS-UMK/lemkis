#pragma once

#include <memory>

#include "node.hpp"

template <typename T>
class queue {
  private:
    std::unique_ptr<node<T>> front_;
    node<T>* rear_ = nullptr;
    std::size_t size_ = 0;

  public:
    queue() = default;

    ~queue() = default;

    auto push(T value) -> void {
        auto new_node = std::make_unique<node<T>>(std::move(value));

        if (empty()) {
            front_ = std::move(new_node);
            rear_ = front_.get();
        } else {
            rear_->next = std::move(new_node);
            rear_ = rear_->next.get();
        }

        size_++;
    }

    auto pop() -> void {

        if (front_.get() == rear_) { rear_ = nullptr; }

        front_ = std::move(front_->next);
        size_--;
    }

    auto front() -> T& { return front_->data; }

    auto front() const -> const T& { return front_->data; }

    auto back() -> T& { return rear_->data; }

    auto back() const -> const T& { return rear_->data; }

    [[nodiscard]] auto empty() const -> bool { return front_ == nullptr; }

    [[nodiscard]] auto size() const -> std::size_t { return size_; }
};