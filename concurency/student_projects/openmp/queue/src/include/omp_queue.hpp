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

    auto push(const T& item) -> void{
        omp_set_lock(&lock);
        queue.push(item);
        omp_unset_lock(&lock);
    }

    auto pop(T& result) -> bool{
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

    auto empty() -> bool {
        omp_set_lock(&lock);
        bool isEmpty = queue.empty();
        omp_unset_lock(&lock);
        return isEmpty;
    }

    auto size() -> size_t {
        omp_set_lock(&lock);
        size_t s = queue.size();
        omp_unset_lock(&lock);
        return s;
    }
};

#endif  // OMP_QUEUE_HPP
