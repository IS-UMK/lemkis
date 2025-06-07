# Project: Benchmarking Concurrent Queues and Parallel Operations in C++

## Project Goal
Celem projektu było zaimplementowanie i porównanie różnych implementacji kolejek współbieżnych oraz benchmarkowanie wydajności równoległych operacji na dużych zbiorach danych. Projekt skupia się na:

- Porównaniu niestandardowej kolejki współbieżnej zaimplementowanej z użyciem `std::jthread` i synchronizacji za pomocą mutexów z prostszą kolejką opartą na OpenMP.
- Benchmarkowaniu operacji wektorowych takich jak transformacje i iloczyny skalarnie, realizowanych różnymi metodami równoległymi (standardowe algorytmy C++ i OpenMP).
- Automatyzacji procesu budowy, testowania i kontroli jakości kodu przy pomocy narzędzi takich jak `clang-format` i `clang-tidy`.

## What Was Done?

### Queue Implementations
- **concurrent_queue**: Niestandardowa kolejka bezpieczna wątkowo, wykorzystująca mutexy i zmienne warunkowe, zarządzająca węzłami przy pomocy `std::unique_ptr` dla bezpiecznego zarządzania pamięcią.
- **omp_queue**: Prostsza kolejka opakowująca `std::queue`, chroniona blokadami OpenMP (`omp_lock_t`).

### Benchmarks

#### Concurrent Queue Benchmark
- Testy z różną liczbą wątków producentów i konsumentów.
- Pomiar czasu operacji push/pop oraz rozmiaru kolejki.
- Użycie uniwersalnej funkcji pomocniczej `measure_and_print` do zbierania i wyświetlania wyników.

#### Vectorized Operation Benchmark (Transform and Dot Product)
- Benchmarki sekwencyjne z wykorzystaniem `std::transform` i `std::inner_product`.
- Benchmarki równoległe z użyciem standardowych algorytmów C++ z politykami wykonania `std::execution::par` oraz `par_unseq`.
- Wersje równoległe z OpenMP, wykorzystujące pętle parallel for i redukcje.
- Testy na małych (10 000 elementów) i dużych (300 milionów elementów) zbiorach danych.

### Automation
- Skrypt bash do:
  - Czyszczenia i budowania projektu przy pomocy `cmake` i `make`.
  - Formatowania kodu zgodnie z `clang-format`.
  - Analizy statycznej i automatycznego poprawiania kodu za pomocą `clang-tidy`.

## Methods and Technologies Used

### Multithreading and Synchronization
- `std::jthread` oraz zmienne atomowe do zarządzania i sygnalizacji wątków.
- Mutexy i zmienne warunkowe dla bezpiecznych operacji na kolejce w `concurrent_queue`.
- Blokady OpenMP oraz dyrektywy pragmatu do synchronizacji i pętli równoległych w `omp_queue` i benchmarkach.

### C++ Parallel Algorithms (C++17/20)
- `std::transform` z politykami wykonania: sekwencyjnym, równoległym i równoległym niesekwencyjnym.
- `std::transform_reduce` do równoległego obliczania iloczynu skalarnego.

### OpenMP
- Pętle równoległe z klauzulami redukcji dla sumowania.
- Kolejka wątkowo bezpieczna z blokadami OpenMP.

### Code Quality and Automation Tools
- `clang-format` do spójnego formatowania kodu.
- `clang-tidy` do analizy statycznej i automatycznych poprawek.

## Summary
Projekt dostarczył praktycznych informacji na temat różnych technik współbieżności i równoległości w nowoczesnym C++. Niestandardowa kolejka z mutexami została porównana z rozwiązaniem opartym na OpenMP, ukazując kompromisy między złożonością implementacji a wydajnością. Benchmarki operacji wektorowych pokazały wpływ różnych polityk wykonania i podejścia OpenMP na wydajność przy różnych rozmiarach danych.

Automatyzacja budowy i kontroli jakości kodu zapewnia łatwość utrzymania i rozwijania projektu w przyszłości.
