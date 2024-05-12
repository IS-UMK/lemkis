Biblioteka Number Theory
Wstęp
Biblioteka Number Theory jest zestawem funkcji i algorytmów związanych z teorią liczb, zaprojektowanym do użytku w programach C++.

Spis treści
Instalacja
Funkcje
Przykłady użycia
Zalecenia


Instalacja
Aby używać tej biblioteki, dołącz pliki nagłówkowe do swojego projektu C++.

Funkcje
Biblioteka zawiera następujące funkcje:

Sprawdzanie, czy zestaw liczb jest względnie pierwszy (co-prime).
Obliczanie potęgi modularnej liczby.
Obliczanie odwrotności modularnej liczby.
Rozwiązywanie równań kongruencji liniowej.
Implementacja Sita Eratostenesa do generowania liczb pierwszych.
Rozkładanie liczby na czynniki pierwsze.
Obliczanie funkcji Eulera phi (funkcji totient).
Obliczanie największej potęgi liczby pierwszej dzielącej silnię danej liczby.
Rozwiązywanie układów równań kongruencji liniowych.

Przykłady użycia
#include "number_theory.hpp"
#include <iostream>

int main() {
    // Obliczanie największego wspólnego dzielnika
    std::cout << number_theory::gcd(24, 36) << std::endl; // Wypisze: 12

    // Sprawdzanie czy zestaw liczb jest względnie pierwszy
    std::vector<std::size_t> numbers = {15, 28, 37};
    std::cout << std::boolalpha << number_theory::are_coprime(numbers) << std::endl; // Wypisze: true

    // Obliczanie potęgi modularnej liczby
    std::cout << number_theory::modular_pow<10>(2, 3) << std::endl; // Wypisze: 8

    // Rozwiązywanie równań kongruencji liniowej
    std::cout << number_theory::linear_congruence_solver<13>(3, 5) << std::endl; // Wypisze: 8

    // Generowanie liczb pierwszych za pomocą Sita Eratostenesa
    auto primes = number_theory::sieve_of_eratosthenes(50);
    for (auto prime : primes) {
        std::cout << prime << " ";
    }
    // Wypisze: 2 3 5 7 11 13 17 19 23 29 31 37 41 43 47

    return 0;
}

Zalecenia
Przed użyciem funkcji upewnij się, że Twoje liczby są typu całkowitego.
W przypadku funkcji operujących na liczbach modulo, zadbaj o przekazanie odpowiednich wartości dla parametrów.
