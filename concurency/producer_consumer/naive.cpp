#include <print>
#include <queue>
#include <thread>

std::queue<int> bufor;
const int MAX_SIZE = 5;

void producent() {
    for (int i = 0; i < 20; ++i) {
        while (bufor.size() >= MAX_SIZE) {}
        bufor.push(i);
        std::println("Wyprodukowano: {}", i);
    }
}

void konsument() {
    for (int i = 0; i < 20; ++i) {
        while (bufor.empty()) {}
        int val = bufor.front();
        bufor.pop();
        std::println("Skonsumowano: {}", val);
    }
}

int main() {
    std::thread p(producent);
    std::thread k(konsument);
    p.join();
    k.join();
}