#include <condition_variable>
#include <iostream>
#include <mutex>
#include <print>
#include <string>
#include <thread>


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