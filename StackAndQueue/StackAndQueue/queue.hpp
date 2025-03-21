#include <format>
#include <iostream>
#include <stdexcept>

template <typename T>
class Queue {
  private:
    struct Node {
        T data;
        Node* next;

        Node(const T& value) : data(value), next(nullptr) {}
    };

    Node* front_ = nullptr;
    Node* rear_ = nullptr;
    std::size_t size_ = 0;

  public:
    // Constructor
    Queue() = default;

    // Destructor
    ~Queue() {
        while (!empty()) { dequeue(); }
    }

    // Copy constructor
    Queue(const Queue& other) {
        Node* current = other.front_;
        while (current != nullptr) {
            enqueue(current->data);
            current = current->next;
        }
    }

    // Move constructor
    Queue(Queue&& other) noexcept
        : front_(other.front_), rear_(other.rear_), size_(other.size_) {
        other.front_ = nullptr;
        other.rear_ = nullptr;
        other.size_ = 0;
    }

    // Copy assignment operator
    Queue& operator=(const Queue& other) {
        if (this != &other) {
            // Clear current queue
            while (!empty()) { dequeue(); }

            // Copy elements from other queue
            Node* current = other.front_;
            while (current != nullptr) {
                enqueue(current->data);
                current = current->next;
            }
        }
        return *this;
    }

    // Move assignment operator
    Queue& operator=(Queue&& other) noexcept {
        if (this != &other) {
            // Clear current queue
            while (!empty()) { dequeue(); }

            front_ = other.front_;
            rear_ = other.rear_;
            size_ = other.size_;

            other.front_ = nullptr;
            other.rear_ = nullptr;
            other.size_ = 0;
        }
        return *this;
    }

    // Add an element to the queue
    void enqueue(const T& value) {
        auto newNode = new Node(value);

        if (empty()) {
            front_ = newNode;
            rear_ = newNode;
        } else {
            rear_->next = newNode;
            rear_ = newNode;
        }

        size_++;
    }

    // Remove and return the front element
    T dequeue() {
        if (empty()) { throw std::underflow_error("Queue is empty"); }

        auto temp = front_;
        T value = temp->data;

        front_ = front_->next;
        delete temp;
        size_--;

        if (front_ == nullptr) { rear_ = nullptr; }

        return value;
    }

    // View the front element without removing it
    const T& peek() const {
        if (empty()) { throw std::underflow_error("Queue is empty"); }
        return front_->data;
    }

    // Check if the queue is empty
    [[nodiscard]] bool empty() const { return front_ == nullptr; }

    // Get the size of the queue
    [[nodiscard]] std::size_t size() const { return size_; }
};