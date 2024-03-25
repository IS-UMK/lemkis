#include <iostream>
#include <format>
#include <print>
#include <numeric>

//int gcd(int a, int b) {
//    while (b != 0) {
//        int temp = b;
//        std::cout << "(" << a << ", " << b << ") = \n";
//        b = a % b;
//        std::cout << "Reszta z dzielenia: " << b << "\n";
//        a = temp;
//    }
//    return a;
//}

void tablica() {
    int n;
    std::cout << "Podaj dla ilu liczb chcesz policzyc NWD: ";
    std::cin >> n;

    int* numbers = new int[n];

    std::cout << "Podaj " << n << " liczb:\n";
    for (int i = 0; i < n; i++) {
        std::cout << "Liczba " << (i + 1) << ": ";
        std::cin >> numbers[i];
    }

    int result = numbers[0];
    for (int i = 1; i < n; i++) {
        result = std::gcd(result, numbers[i]);
    }

    std::cout << "NWD podanych liczb to: " << result << std::endl;

    delete[] numbers;
}

int main() {
    auto a = std::gcd(7, 14);
    std::println("Wartosc a: {}", a);
    tablica();

    return 0;
}