#pragma once

#include <atomic>

inline auto atomic_multiply_example(std::atomic<double>& x, double m) {
    double x0{x};
    // compare_exchange_strong: This is a member function of std::atomic that
    // performs a strong compare-exchange operation. It checks if the current
    // value of x is equal to the expected value (x0), and if so, it replaces
    // the value of x with the new value (x0 * m).  If the exchange fails, the
    // loop repeats, updating x0 with the current value of x and recalculating
    // the new value
    while (!x.compare_exchange_strong(x0, x0 * m)) {}
}


template <typename T>
struct node {
    T data{};
    node* next{nullptr};
};

template <typename T>
class stack {
public:
    std::atomic<node<T>*> head;

  public:
    void push(const T& data) {
        node<T>* new_node = new node<T>(data);
        new_node->next = head.load(std::memory_order_relaxed);
        while (!head.compare_exchange_weak(new_node->next,
                                           new_node,
                                           std::memory_order_release,
                                           std::memory_order_relaxed)) {}
    }
};

