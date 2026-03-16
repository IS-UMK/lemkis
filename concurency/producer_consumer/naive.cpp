#include <iostream>
#include <queue>
#include <thread>

std::queue<int> bufor;
const int MAX_SIZE = 5;

void producent() {
    for (int i = 0; i < 20; ++i) {
        while (bufor.size() >= MAX_SIZE) {}
        bufor.push(i);
        std::cout << "Wyprodukowano: " << i << "\n";
    }
}

void konsument() {
    for (int i = 0; i < 20; ++i) {
        while (bufor.empty()) {}
        int val = bufor.front();
        bufor.pop();
        std::cout << "Skonsumowano: " << val << "\n";
    }
}

int main() {
    std::thread p(producent);
    std::thread k(konsument);
    p.join();
    k.join();
}