# Dokumentacja projektu basic_algebra_2_pack

## Przegląd

Projekt basic_algebra_2_pacl dostarcza zestaw funkcji do wykonywania różnych operacji na macierzach oraz rozwiązywania problemów związanych z równaniami liniowymi i przestrzeniami liniowymi. Zawiera szereg struktur danych i algorytmów zaimplementowanych w oparciu o pliki matrix.hpp oraz gaussian_elimination.hpp

## Moduły

### algebra

Moduł `algebra` zawiera główne funkcje do rozwiązywania równań liniowych, obliczania jądra macierzy, sprawdzania przynależności wektorów do przestrzeni liniowej oraz inne operacje związane z algebrą liniową.

#### 1.1 matrix

```javascript
// Ta część modułu definiuje typ `matrix`, który jest używany do reprezentacji macierzy w postaci pary: wektor danych oraz widoku macierzy.

### 1.2 homogeneous_solution_t

// Typ `homogeneous_solution_t` jest używany do reprezentowania rozwiązania jednorodnego równań liniowych.

### 1.3 to_fraction_type

// Typ `to_fraction_type` jest używany do konwersji wartości na ułamek w przypadku potrzeby.

### 1.4 first_non_zero

// Funkcja `first_non_zero` znajduje pierwszy niezerowy element w określonym wierszu macierzy.

### 1.5 is_contradictory

// Funkcja `is_contradictory` sprawdza, czy istnieje sprzeczność w macierzy.

### 1.6 pivots

// Funkcja `pivots` znajduje indeksy pierwszych niezerowych elementów (pivots) w każdym wierszu macierzy.

### 1.7 special_solution

// Funkcja `special_solution` oblicza rozwiązanie szczególne systemu równań liniowych.

### 1.8 solve

// Funkcja `solve` rozwiązuje system równań liniowych.

### 1.9 kernel_3, 1.10 kernel_2, 1.11 kernel

// Funkcje `kernel_3`, `kernel_2`, `kernel` są częściami procesu obliczania jądra macierzy.

### 1.12 is_in_span

// Funkcja `is_in_span` sprawdza, czy dany wektor należy do przestrzeni rozpiętej przez kolumny danej macierzy.

### 1.13 forms_base

// Funkcja `forms_base` sprawdza, czy zbiór wektorów tworzy bazę przestrzeni liniowej.

### 1.14 coordinates_in_base

// Funkcja `coordinates_in_base` oblicza współrzędne danego wektora względem danej bazy.

### 1.15 matrix_multiply

// Funkcja `matrix_multiply` wykonuje mnożenie macierzy.

### 1.16 base_transition_matrix

// Funkcja `base_transition_matrix` oblicza macierz przejścia między dwiema bazami przestrzeni liniowej.

### 1.17 is_in_kernel, 1.18 is_in_image

// Funkcje `is_in_kernel`, `is_in_image` sprawdzają, czy dany wektor należy odpowiednio do jądra danej macierzy lub obrazu danej przekształcenia liniowego.

## Przykłady użycia

Ten moduł zawiera przykłady użycia głównych funkcji z modułu algebra w celu demonstracji ich działania.

## Kompatybilność

Kod jest kompatybilny z kompilatorami obsługującymi koncepcje i funkcje C++20.

## Uwaga

Ta dokumentacja zapewnia przegląd projektu Algebraic Operations i jego składników. Zachęca się użytkowników do odwołania się do kodu źródłowego i komentarzy w celu uzyskania szczegółowych informacji i szczegółów implementacji.