#include <iostream>
#include <thread>
#include <vector>
#include <atomic>
#include <chrono>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <optional>

template<typename T>
class BoundedBuffer {
    std::queue<T> queue_;
    const size_t max_size_;
    std::mutex mutex_;
    std::condition_variable cv_not_full_;
    std::condition_variable cv_not_empty_;
    bool shutdown_ = false;

public:
    explicit BoundedBuffer(size_t max_size) : max_size_(max_size) {}

    // Zwraca false, jeśli bufor zamknięty
    bool push(const T& item) {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_not_full_.wait(lock, [this] {
            return queue_.size() < max_size_ || shutdown_;
        });
        if (shutdown_) return false;

        queue_.push(item);
        cv_not_empty_.notify_one();
        return true;
    }

    // Zwraca std::nullopt, jeśli bufor zamknięty i pusty
    std::optional<T> pop() {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_not_empty_.wait(lock, [this] {
            return !queue_.empty() || shutdown_;
        });
        if (queue_.empty()) return std::nullopt;

        T item = std::move(queue_.front());
        queue_.pop();
        cv_not_full_.notify_one();
        return item;
    }

    void close() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            shutdown_ = true;
        }
        cv_not_full_.notify_all();
        cv_not_empty_.notify_all();
    }
};



int main() {
    BoundedBuffer<int> buf(10);

    const int NUM_PRODUCERS = 3;
    const int NUM_CONSUMERS = 2;

    std::atomic<int> produced{0};
    std::atomic<int> consumed{0};

    // --- Producenci (jthread + stop_token) ---
    std::vector<std::jthread> producers;
    for (int pid = 0; pid < NUM_PRODUCERS; ++pid) {
        producers.emplace_back(
            [&buf, &produced, pid](std::stop_token st) {
                int value = pid * 1000000;
                while (!st.stop_requested()) {
                    // jakaś „produkcja”
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));

                    // próba wstawienia do bufora
                    if (!buf.push(value)) {
                        // bufor zamknięty → kończymy
                        break;
                    }
                    ++produced;
                    ++value;
                }
                // tu ewentualnie dokończenie „w locie”
            }
        );
    }

    // --- Konsumenci (jthread + stop_token) ---
    std::vector<std::jthread> consumers;
    for (int cid = 0; cid < NUM_CONSUMERS; ++cid) {
        consumers.emplace_back(
            [&buf, &consumed, cid](std::stop_token st) {
                for (;;) {
                    if (st.stop_requested()) {
                        // opcjonalnie: można jeszcze spróbować „spuścić” bufor do końca
                        break;
                    }

                    auto item = buf.pop();
                    if (!item) {
                        // bufor zamknięty i pusty → koniec
                        break;
                    }

                    // „konsumpcja”
                    ++consumed;
                    // np. std::cout << "C" << cid << " -> " << *item << "\n";
                }
            }
        );
    }

    // --- Symulacja pracy systemu ---
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // 1) prosimy wątki o zatrzymanie przez stop_token
    for (auto& t : producers)
        t.request_stop();
    for (auto& t : consumers)
        t.request_stop();

    // 2) zamykamy bufor, żeby obudzić tych, którzy śpią na wait()
    buf.close();

    // 3) destruktory std::jthread zrobią join()

    std::cout << "Wyprodukowano: " << produced.load() << "\n";
    std::cout << "Skonsumowano:  " << consumed.load() << "\n";
}