#include <vector>
#include <mutex>
#include <condition_variable>
#include <cassert>

template<typename T>
class ring_buffer {
     std::vector<T> data_;
     size_t head_ = 0;          // indeks do zapisu (producent)
     size_t tail_ = 0;          // indeks do odczytu (konsument)
     size_t count_ = 0;         // liczba elementów
     size_t capacity_;

     std::mutex mutex_;
     std::condition_variable cv_not_full_;
     std::condition_variable cv_not_empty_;

public:
     explicit ring_buffer(size_t capacity)
         : data_(capacity), capacity_(capacity) {}

     void push(const T& item) {
         std::unique_lock<std::mutex> lock(mutex_);
         cv_not_full_.wait(lock, [this] { return count_ < capacity_; });

         data_[head_] = item;
         head_ = (head_ + 1) % capacity_;
         ++count_;

         cv_not_empty_.notify_one();
     }

     T pop() {
         std::unique_lock<std::mutex> lock(mutex_);
         cv_not_empty_.wait(lock, [this] { return count_ > 0; });

         T item = std::move(data_[tail_]);
         tail_ = (tail_ + 1) % capacity_;
         --count_;

         cv_not_full_.notify_one();
         return item;
     }
};