#include <iostream>
#include <mutex>
#include <queue>
#include <thread>

std::queue<int> bufor;
const int MAX_SIZE = 5;
std::mutex mtx;

void producent() {
    for (int i = 0; i < 20; ++i) {
        while (true) {
            std::lock_guard<std::mutex> lock(mtx);
            if (bufor.size() < MAX_SIZE) {
                bufor.push(i);
                std::cout << "Wyprodukowano: " << i << "\n";
                break;
            }
        }
    }
}

void konsument() {
    for (int i = 0; i < 20; ++i) {
        while (true) {
            std::lock_guard<std::mutex> lock(mtx);
            if (!bufor.empty()) {
                int val = bufor.front();
                bufor.pop();
                std::cout << "Skonsumowano: " << val << "\n";
                break;
            }
        }
    }
}

int main() {
    std::thread p(producent);
    std::thread k(konsument);
    p.join();
    k.join();
}