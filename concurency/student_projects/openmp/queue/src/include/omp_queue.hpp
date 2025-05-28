#ifndef OMP_QUEUE_HPP
#define OMP_QUEUE_HPP

#include <cstddef>
#include <queue>

// Proper OpenMP detection and inclusion
#if defined(_OPENMP)
#include <omp.h>
#else
// Only trigger the static assert if someone actually tries to use this header
// without OpenMP support
#define OMP_QUEUE_NO_OPENMP
#endif

template <typename T>
class omp_queue {
  private:
    std::queue<T> queue;
#ifdef OMP_QUEUE_NO_OPENMP
    // Dummy lock type when OpenMP isn't available
    struct dummy_lock {};
    dummy_lock lock;
#else
    omp_lock_t lock;
#endif
    bool is_initialized = false;

  public:
    omp_queue() {
#ifdef OMP_QUEUE_NO_OPENMP
        static_assert(false, "OpenMP support is required for this header");
#else
        omp_init_lock(&lock);
        is_initialized = true;
#endif
    }

    ~omp_queue() {
#ifndef OMP_QUEUE_NO_OPENMP
        if (is_initialized) { omp_destroy_lock(&lock); }
#endif
    }

    // Delete copy constructor and assignment operator
    omp_queue(const omp_queue&) = delete;
    auto operator=(const omp_queue&) -> omp_queue& = delete;

    // Move constructor
    omp_queue(omp_queue&& other) noexcept {
#ifndef OMP_QUEUE_NO_OPENMP
        std::swap(queue, other.queue);
        std::swap(lock, other.lock);
        std::swap(is_initialized, other.is_initialized);
#endif
    }

    // Move assignment operator
    auto operator=(omp_queue&& other) noexcept -> omp_queue& {
#ifndef OMP_QUEUE_NO_OPENMP
        if (this != &other) {
            std::swap(queue, other.queue);
            std::swap(lock, other.lock);
            std::swap(is_initialized, other.is_initialized);
        }
#endif
        return *this;
    }

    auto push(const T& item) -> void {
#ifndef OMP_QUEUE_NO_OPENMP
        omp_set_lock(&lock);
        queue.push(item);
        omp_unset_lock(&lock);
#endif
    }

    [[nodiscard]] auto try_pop(T& result) -> bool {
#ifndef OMP_QUEUE_NO_OPENMP
        omp_set_lock(&lock);
        if (queue.empty()) {
            omp_unset_lock(&lock);
            return false;
        }
        result = queue.front();
        queue.pop();
        omp_unset_lock(&lock);
        return true;
#else
        return false;
#endif
    }

    [[nodiscard]] auto empty() const -> bool {
#ifndef OMP_QUEUE_NO_OPENMP
        omp_set_lock(&lock);
        const bool result = queue.empty();
        omp_unset_lock(&lock);
        return result;
#else
        return true;
#endif
    }

    [[nodiscard]] auto size() const -> size_t {
#ifndef OMP_QUEUE_NO_OPENMP
        omp_set_lock(&lock);
        const size_t result = queue.size();
        omp_unset_lock(&lock);
        return result;
#else
        return 0;
#endif
    }
};

#endif  // OMP_QUEUE_HPP