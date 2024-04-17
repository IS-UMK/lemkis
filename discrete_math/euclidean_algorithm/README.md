# Dokumentacja projektu: Rozszerzony Algorytm Euklidesa

## Przegląd

Projekt Rozszerzonego Algorytmu Euklidesa zapewnia funkcjonalności związane z
obliczaniem współczynników oraz największego wspólnego dzielnika (GCD) dla par i zbiorów liczb całkowitych.
Biblioteka zawiera także funkcje do wyjaśniania kolejnych kroków algorytmu euklidesowego
oraz operacje na rozszerzonym algorytmie euklidesowym.

## Moduły

### 1. algorithms

Ten moduł zawiera implementacje podstawowych algorytmów euklidesowych.

#### 1.1 euclidean_result

- Opis: Struktura zawierająca współczynniki i największy wspólny dzielnik.
- Składowe:
  - coefficients: Zawierająca współczynniki.
  - GCD: Największy wspólny dzielnik.

#### 1.2 gcd_extended

- Opis: Funkcja obliczająca współczynniki i największy wspólny dzielnik dla dwóch elementów.
- Zwracane: Para zawierająca wynik oraz kroki wykonane podczas obliczeń.

#### 1.3 gcd_extended (przeciążenie)

- Opis: Funkcja obliczająca współczynniki i największy wspólny dzielnik dla wielu elementów.
- Zwracane: Para zawierająca tablicę współczynników oraz największy wspólny dzielnik.

#### 1.4 euclidean

- Opis: Klasa zawierająca funkcje związane z algorytmem euklidesa.
- Metody:
  - array_to_tuple: Konwertuje tablicę na krotkę.
  - operator(): Oblicza współczynniki i największy wspólny dzielnik dla podanych liczb.
  - showSteps: Wyświetla i wyjaśnia kolejne kroki algorytmu euklidesowego.

### 2. algorithms_tests

Ten moduł zawiera testy sprawdzające poprawność zaimplementowanych funkcji.

#### Użycie

- Użytkownicy mogą dołączyć niezbędne nagłówki z przestrzeni nazw algorithms do pracy z algorytmami euklidesowymi.
- Dostępne są różne operacje, takie jak obliczanie współczynników, największego wspólnego dzielnika oraz wyjaśnianie kroków algorytmu euklidesowego.

#### Przykładowy kod zawieruający możliwość opisania krok po kroku, jak działa algorytm lub zwykłe policzenie NWD oraz Bézout's identity
```cpp
#include "./euclidean.hpp"
#include <iostream>

void menu()
{
    std::println("Choose an option:");
    std::println("1. Show the result of an Extended Euclidean Algorithm.");
    std::println("2. Show steps of completing an Extended Euclidean Algorithm.");
    std::print("Your choice: ");
}

void display_steps()
{
    std::string explaination2 = algorithms::example_for_n_digits(4, 6, 8);
    std::println("Let us show how the steps look like:\n {}", explaination2);

    std::string explaination1 = algorithms::example_for_two_digits(546, 308);
    std::println("Let us show how the steps look like:\n {}", explaination1);
}

void choice_options()
{
    int choice;
    do {
        menu();
        std::cin >> choice;

        switch (choice) {
        case 1:
            algorithms::example_for_n_digits(4, 6, 8);
            algorithms::example_for_two_digits(546, 308);
            break;
        case 2:
            display_steps();
            break;
        default:
            std::println("Wrong choice. Try again.");
            break;
        }
    } while (choice != 1 && choice != 2);
}

int main()
{
    choice_options();
}
```

#### Wyjaśnienie działania algorytmu

- Największy wspólny dzielnik (gcd) dla n liczb całkowitych można obliczyć za pomocą rekurencyjnego podejścia.

    - Jeśli n = 2: W przypadku dwóch liczb, możemy po prostu użyć algorytmu Euklidesa, który polega na iteracyjnym dzieleniu jednej liczby przez drugą,
      aż otrzymamy resztę równą zero. Gdy reszta jest równa zero, ostatnia niezerowa wartość to gcd tych dwóch liczb.

    - Jeśli n > 2: Możemy zastosować rekurencyjne podejście, korzystając z faktu, że gcd jest łączny. Innymi słowy, gcd(a, b, c) = gcd(gcd(a, b), c).
      W oparciu o to, możemy rekurencyjnie obliczać gcd dla mniejszej liczby liczb, aż zostanie nam tylko jedna liczba.

    Zacznijmy od obliczenia gcd(a, b).
    Następnie użyjemy wyniku jako pierwszy argument gcd w kolejnym wywołaniu gcd(gcd(a, b), c).
    Powtarzamy ten proces, aż pozostanie nam tylko jedna liczba, która będzie gcd(a, b, c, ..., n).
    Ostatecznie, obliczenie gcd dla n liczb sprowadza się do obliczenia gcd dla dwóch liczb, a następnie wykorzystania tego wyniku w rekurencyjnych
    wywołaniach gcd dla pozostałych liczb, aż pozostanie tylko jedna liczba. 

- Rozszerzony algorytm Euklidesa służy do znajdowania współczynników Bezouta dla liczb. Współczynniki Bezouta to liczby całkowite x i y, takie że: gcd(a, b) = ax + by.

    - Korzysta on z gcd do "odzyskiwania" konkretnych współczynników. Po znalezieniu gcd(a, b), możemy odzyskać współczynniki Bezouta, korzystając
      z procesu zwrotnego (backtracking), czyli cofania się przez iteracje algorytmu Euklidesa. Podczas cofania się, możemy wyrazić każdą resztę
      jako kombinację liniową a i b, a więc również jako kombinację liniową gcd(a, b), a zatem możemy wyznaczyć współczynniki Bezouta. Zaczynamy od
      ostatniej iteracji algorytmu Euklidesa, w której reszta jest równa gcd(a, b). Następnie, cofając się przez iteracje algorytmu, używając
      równania reszty r = a - q * b, możemy wyrazić każdą resztę jako kombinację liniową a i b. Ostatecznie, gdy cofniemy się do początku algorytmu,
      otrzymamy równanie gcd(a, b) = ax + by, gdzie x i y są współczynnikami Bezouta dla liczb a i b.

 Dla n > 2:
 Gcd(a1, a2, ..., an) = gcd(gcd(a1, a2, ..., an-1), an)
 Zgodnie z tą własnością algorytm rekurencyjnie oblicza NWD wszystkich liczb w ciągu poprzez wywołanie funkcji NWD dla dwóch ostatnich liczb,
 a następnie stosując wynik jako pierwszą liczbę dla kolejnego wywołania funkcji NWD. 
```
      auto [coeff, gcd] = algorithms::gcd_extended(all_but_last);
```
   Tutaj obliczamy NWD dla wszystkich liczb z wyjątkiem ostatniej (all_but_last). Wynik ten zawiera Bézout's coefficients (współczynniki Bézouta)
   oraz NWD tych liczb.
```    
      auto [result2, step2] = algorithms::gcd_extended(gcd, is[size - 1]);
```
   Następnie obliczamy NWD dla ostatniej liczby w ciągu (is[size - 1]) i poprzedniego wyniku NWD (gcd). Wynik ten również zawiera Bézout's
      coefficients oraz NWD tych dwóch liczb.
```
      auto [coeff2, gcd2] = result2;
```

   Wyciągamy Bézout's coefficients i NWD z wyniku obliczenia NWD dla ostatnich dwóch liczb.

```
      auto [alpha, beta] = coeff2;
```

   Wyciągamy współczynniki Bézouta z wyniku obliczenia NWD dla ostatnich dwóch liczb, które są współczynnikami Bézouta dla całego ciągu liczb.
      Te cztery linie kodu implementują rekurencyjną właściwość algorytmu wykorzystującą NWD, która pozwala na obliczenie NWD dla całego ciągu liczb poprzez wykorzystanie NWD dwóch ostatnich liczb oraz wyników poprzednich obliczeń NWD.


- Przykład:

#  Dla 2 liczb gcd(546,308) = 14 -9 * 546 + 16 * 308

    |  546 % 308 = 238   546 = 308 * 1 + 238  ʌ

    |  308 % 238 = 70    308 = 238 * 1 + 70   |

    |  238 % 70 = 28     238 = 70 * 3 + 28    |

    |  70 % 28 = 14      70 = 28 * 2 + 14     |

    V  28 % 14 = 0       28 = 2 * 14          |

   Czyli: wstawiamy równanie niżej do równaina wyżej, np 70 = (14 * 2) * 2 + 14

#### Kompatybilność
* Kod jest kompatybilny z kompilatorami obsługującymi koncepcje i 
funkcje C++20.

### Uwaga
* Zachęca się użytkowników do odwołania się do kodu źródłowego i 
komentarzy w celu uzyskania szczegółowych informacji i szczegółów 
implementacji.