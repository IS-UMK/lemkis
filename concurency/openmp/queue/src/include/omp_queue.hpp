#ifndef OMP_QUEUE_HPP
#define OMP_QUEUE_HPP

#include <omp.h>

#include <cstddef>
#include <queue>

template <typename T>
class OMPQueue {
  private:
    std::queue<T> queue;
    omp_lock_t lock;

  public:
    OMPQueue() { omp_init_lock(&lock); }

    ~OMPQueue() { omp_destroy_lock(&lock); }

    void push(const T& item) {
        omp_set_lock(&lock);
        queue.push(item);
        omp_unset_lock(&lock);
    }

    bool pop(T& result) {
        omp_set_lock(&lock);
        if (!queue.empty()) {
            result = queue.front();
            queue.pop();
            omp_unset_lock(&lock);
            return true;
        }
        omp_unset_lock(&lock);
        return false;
    }

    bool empty() {
        omp_set_lock(&lock);
        bool isEmpty = queue.empty();
        omp_unset_lock(&lock);
        return isEmpty;
    }

    size_t size() {
        omp_set_lock(&lock);
        size_t s = queue.size();
        omp_unset_lock(&lock);
        return s;
    }
};

#endif  // OMP_QUEUE_HPP
