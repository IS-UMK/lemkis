#include <condition_variable>
#include <iostream>
#include <mutex>
#include <print>
#include <string>
#include <thread>
#include <vector>

struct worker {
  public:
    std::mutex m;
    std::condition_variable cv;
    std::string data;
    bool ready = false;
    bool processed = false;

  public:
    void worker_thread() {
        // wait until main() sends data
        std::unique_lock lk(m);
        cv.wait(lk, [this] { return ready; });
        // after the wait, we own the lock
        std::println("Worker thread is processing data");
        data += " after processing";
        // send data back to main()
        processed = true;
        std::println("Worker thread signals data processing completed");
        // manual unlocking is done before notifying, to avoid waking up
        // the waiting thread only to block again (see notify_one for details)
        lk.unlock();
        cv.notify_one();
    }


    auto main_thread() {
        std::jthread worker{[this]() { worker_thread(); }};
        data = "Example data";
        // send data to the worker thread
        {
            std::lock_guard lk(m);
            ready = true;
            std::cout << "we signal data ready for processing\n";
        }
        cv.notify_one();
        // wait for the worker
        {
            std::unique_lock lk(m);
            cv.wait(lk, [this] { return processed; });
        }
        std::println("Worker is done, data = {}", data);
    }
};

struct factory {
    std::mutex m;
    std::condition_variable cv;
    std::vector<int> cookies{};
    int cookie_id{0};
    public:
    void produce_a_cookie() {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        {
            std::lock_guard lk(m);
            cookies.push_back(cookie_id);
            std::println("a cookie {} is ready\n", cookie_id++);
        }
        cv.notify_one();

    }

    void consume_a_cookie() {   
        std::println("cookies = {}", cookies.size());
        {
            std::unique_lock lk(m);
            std::println("between lock and wait"); 
            cv.wait(lk, [this] { return !cookies.empty(); });
            /* tutaj mam mutex */
            int cookie = cookies.back();
            std::println("a cookie {} is acquired\n", cookie);
            cookies.pop_back();
        }
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
    
};



