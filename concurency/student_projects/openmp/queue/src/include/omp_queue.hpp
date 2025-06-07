#ifndef OMP_QUEUE_HPP
#define OMP_QUEUE_HPP

#include <omp.h>

#include <cstddef>
#include <queue>

// A simple thread-safe queue implementation using OpenMP locks.
// Wraps std::queue and synchronizes access via omp_lock_t.
template <typename T>
class omp_queue {
  private:
    std::queue<T> queue;  // Standard queue as the underlying container
    omp_lock_t lock;      // OpenMP lock for mutual exclusion
    bool created;         // Indicates whether the lock was initialized

  public:
    // Constructor initializes the OpenMP lock
    omp_queue() : lock() {
        created = true;
        omp_init_lock(&lock);
    }

    // Destructor destroys the lock to prevent resource leaks
    ~omp_queue() {
        created = false;
        omp_destroy_lock(&lock);
    }

    // Adds an item to the queue in a thread-safe way
    void push(const T& item) {
        omp_set_lock(&lock);
        queue.push(item);
        omp_unset_lock(&lock);
    }

    // Attempts to remove the front item.
    // Returns true if an element was removed, false if queue was empty.
    auto pop() -> bool {
        omp_set_lock(&lock);
        bool has_element = !queue.empty();
        if (has_element) { queue.pop(); }
        omp_unset_lock(&lock);
        return has_element;
    }

    // Returns true if the queue is empty (thread-safe check)
    auto empty() -> bool {
        omp_set_lock(&lock);
        bool is_empty = queue.empty();
        omp_unset_lock(&lock);
        return is_empty;
    }

    // Returns the number of elements in the queue
    auto size() -> size_t {
        omp_set_lock(&lock);
        size_t s = queue.size();
        omp_unset_lock(&lock);
        return s;
    }
};

#endif  // OMP_QUEUE_HPP
