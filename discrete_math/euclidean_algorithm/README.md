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

#### Przykład
```

    ExampleForNDigits(4, 6, 8);
    ExampleForTwoDigits(546, 308);
```

#### Kompatybilność
* Kod jest kompatybilny z kompilatorami obsługującymi koncepcje i 
funkcje C++20.

### Uwaga
* Zachęca się użytkowników do odwołania się do kodu źródłowego i 
komentarzy w celu uzyskania szczegółowych informacji i szczegółów 
implementacji.