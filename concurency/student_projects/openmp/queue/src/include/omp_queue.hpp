#ifndef OMP_QUEUE_HPP
#define OMP_QUEUE_HPP

#include <omp.h>

#include <cstddef>
#include <queue>

template <typename T>
class omp_queue {
  private:
    std::queue<T> queue;
    omp_lock_t lock;

  public:
    omp_queue() { omp_init_lock(&lock); }

    ~omp_queue() { omp_destroy_lock(&lock); }

    void push(const T& item) {
        omp_set_lock(&lock);
        queue.push(item);
        omp_unset_lock(&lock);
    }
    auto pop(T& result) -> bool {
        omp_set_lock(&lock);
        return !queue.empty();
    }
    auto empty() -> bool {
        omp_set_lock(&lock);
        bool is_empty = queue.empty();
        omp_unset_lock(&lock);
        return is_empty;
    }
    auto size() -> size_t {
        omp_set_lock(&lock);
        size_t s = queue.size();
        omp_unset_lock(&lock);
        return s;
    }
};

#endif  // OMP_QUEUE_HPP