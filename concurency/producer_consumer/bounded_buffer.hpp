#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <vector>
#include <atomic>
#include <cassert>

template<typename T>
class bounded_buffer {
     std::queue<T> queue_;
     const size_t max_size_;

     std::mutex mutex_;
     std::condition_variable cv_not_full_;   // producenci czekają na to
     std::condition_variable cv_not_empty_;  // konsumenci czekają na to

public:
     explicit bounded_buffer(size_t max_size) : max_size_(max_size) {
         assert(max_size > 0);
     }

     // Wstawia element do bufora. Blokuje, jeśli bufor jest pełny.
     void push(const T& item) {
         std::unique_lock<std::mutex> lock(mutex_);

         // Czekaj aż bufor nie będzie pełny
         //
         // wait() robi trzy rzeczy:
         //   1. Sprawdza predykat — jeśli true, nie czeka
         //   2. Jeśli false: ATOMOWO zwalnia mutex i usypia wątek
         //   3. Po obudzeniu: ponownie blokuje mutex i sprawdza predykat
         //
         // Dlaczego predykat, a nie zwykłe wait()?
         //   → Ochrona przed SPURIOUS WAKEUP (fałszywe obudzenie)
         //   → System operacyjny MOŻE obudzić wątek bez notify!
         cv_not_full_.wait(lock, [this] {
             return queue_.size() < max_size_;
         });

         queue_.push(item);

         // Budzenie JEDNEGO czekającego konsumenta
         cv_not_empty_.notify_one();
     }

     // Pobiera element z bufora. Blokuje, jeśli bufor jest pusty.
     T pop() {
         std::unique_lock<std::mutex> lock(mutex_);

         // Czekaj aż bufor nie będzie pusty
         cv_not_empty_.wait(lock, [this] {
             return !queue_.empty();
         });

         T item = std::move(queue_.front());
         queue_.pop();

         // Budzenie JEDNEGO czekającego producenta
         cv_not_full_.notify_one();

         return item;
     }
};
