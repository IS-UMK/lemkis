#pragma once

#include <memory>

#include "node.hpp"

template <typename T>
class Queue {
  private:
    std::unique_ptr<Node<T>> front_;
    Node<T>* rear_ = nullptr;
    std::size_t size_ = 0;

  public:
    Queue() = default;
    ~Queue() = default;
    Queue(const Queue& other) {
        Node<T>* current = other.front_.get();
        while (current != nullptr) {
            push(current->data);
            current = current->next.get();
        }
    }
    Queue(Queue&& other) noexcept
        : front_(std::move(other.front_)),
          rear_(other.rear_),
          size_(other.size_) {
        other.rear_ = nullptr;
        other.size_ = 0;
    }
    auto operator=(const Queue& other) -> Queue& {
        if (this != &other) {
            front_.reset();
            rear_ = nullptr;
            size_ = 0;
            Node<T>* current = other.front_.get();
            while (current != nullptr) {
                push(current->data);
                current = current->next.get();
            }
        }
        return *this;
    }
    auto operator=(Queue&& other) noexcept -> Queue& {
        if (this != &other) {
            front_ = std::move(other.front_);
            rear_ = other.rear_;
            size_ = other.size_;

            other.rear_ = nullptr;
            other.size_ = 0;
        }
        return *this;
    }
    auto push(T value) -> void {
        auto newNode = std::make_unique<Node<T>>(std::move(value));

        if (empty()) {
            front_ = std::move(newNode);
            rear_ = front_.get();
        } else {
            rear_->next = std::move(newNode);
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