#include <print>
#include <thread>
#include <atomic>

std::atomic<bool> wants[2] = {false, false};
std::atomic<int>  who_waits{1};

void process(int id) {
    int other = 1 - id;
    while (true) {
        // own_stuff();
        std::this_thread::sleep_for(std::chrono::seconds(std::rand() % 3));

        wants[id] = true;          // deklaruję chęć wejścia
        who_waits = id;            // ustępuję: "ja mogę poczekać"
        while (wants[other] && who_waits == id) {
            /* busy wait */
        }
        std::println("Thread {} enters the critical section", id);
        // --- sekcja krytyczna ---
        std::println("Thread {} leaves the critical section", id);
        wants[id] = false;         // opuszcza sekcję krytyczną
    }
}

int main() {
    std::thread t1(process, 0);
    std::thread t2(process, 1);

    t1.join();
    t2.join();

    return 0;
}