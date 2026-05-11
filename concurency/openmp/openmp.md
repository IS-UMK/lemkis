# OpenMP w C++ — od podstaw do zaawansowanych technik
## Notatki do wykładu/ćwiczeń z programowania współbieżnego

---

## Spis treści

1. [Czym jest OpenMP?](#1-czym-jest-openmp)
2. [Model programowania i kompilacja](#2-model-programowania-i-kompilacja)
3. [Podstawowe dyrektywy — `#pragma omp parallel`](#3-podstawowe-dyrektywy--pragma-omp-parallel)
4. [Klauzule danych](#4-klauzule-danych)
5. [Work-sharing — podział pracy](#5-work-sharing--podział-pracy)
6. [Synchronizacja](#6-synchronizacja)
7. [Zarządzanie wątkami — API OpenMP](#7-zarządzanie-wątkami--api-openmp)
8. [Klauzule `schedule` — strategie podziału iteracji](#8-klauzule-schedule--strategie-podziału-iteracji)
9. [Reduction — redukcja równoległa](#9-reduction--redukcja-równoległa)
10. [Zaawansowane OpenMP — Tasks](#10-zaawansowane-openmp--tasks)
11. [SIMD — wektoryzacja](#11-simd--wektoryzacja)
12. [Zagnieżdżona równoległość](#12-zagnieżdżona-równoległość)
13. [Model pamięci OpenMP](#13-model-pamięci-openmp)
14. [Offloading na GPU / akcelerator (`target`)](#14-offloading-na-gpu--akcelerator-target)
15. [Pułapki i dobre praktyki](#15-pułapki-i-dobre-praktyki)
16. [Porównanie: OpenMP vs `std::thread` vs TBB](#16-porównanie-openmp-vs-stdthread-vs-tbb)

---

## 1. Czym jest OpenMP?

**OpenMP** (Open Multi-Processing) to **API do programowania równoległego**
z pamięcią współdzieloną, oparte na dyrektywach preprocesora (`#pragma`),
funkcjach bibliotecznych i zmiennych środowiskowych.

### Kluczowe cechy

| Cecha | Opis |
|-------|------|
| **Przenośność** | C, C++, Fortran; Linux, Windows, macOS |
| **Model fork-join** | Główny wątek rozgałęzia się w region równoległy, a potem łączy |
| **Incremental parallelism** | Dodajesz `#pragma` do istniejącego kodu — nie piszesz od nowa |
| **Automatyczne zarządzanie wątkami** | Biblioteka tworzy/niszczy wątki za Ciebie |
| **Wspólna pamięć** | Wątki widzą te same zmienne; synchronizacja manualna |

### Model fork-join

```
Główny wątek
──────────────────────────────────────► t
              │                    │
              ▼ fork               ▼ join
         ┌────────────────────┐
         │  Wątek 0  (master) │
         │  Wątek 1           │   region równoległy
         │  Wątek 2           │
         │  Wątek 3           │
         └────────────────────┘
              (implicit barrier)
```

```cpp
// Uproszczony schemat
int main() {
    // --- kod szeregowy ---

    #pragma omp parallel   // ← FORK: biblioteka tworzy N wątków
    {
        // --- kod równoległy ---
        // każdy wątek wykonuje ten blok
    }                      // ← JOIN: wątki czekają na barierze

    // --- kod szeregowy ---
}
```

---

## 2. Model programowania i kompilacja

### 2.1 Kompilacja

```bash
# GCC
g++ -O2 -fopenmp plik.cpp -o program

# Clang
clang++ -O2 -fopenmp plik.cpp -o program

# MSVC
cl /openmp plik.cpp
```

> **Bez flagi `-fopenmp`** dyrektywy `#pragma omp ...` są **ignorowane** przez kompilator
> — program działa szeregowo. To cenna właściwość: ten sam kod może być szeregowy lub równoległy!

### 2.2 Niezbędne nagłówki

```cpp
#include <omp.h>    // funkcje API: omp_get_thread_num(), omp_get_num_threads() itp.
// Dyrektywy #pragma NIE wymagają nagłówka — są przetwarzane przez preprocesor
```

### 2.3 Sprawdzenie dostępności OpenMP

```cpp
#include <iostream>

int main() {
#ifdef _OPENMP
    std::cout << "OpenMP wersja: " << _OPENMP << "\n";
    // _OPENMP = 201811 → OpenMP 5.0
    // _OPENMP = 201611 → OpenMP 4.5
    // _OPENMP = 201307 → OpenMP 4.0
#else
    std::cout << "OpenMP niedostępny\n";
#endif
}
```

---

## 3. Podstawowe dyrektywy — `#pragma omp parallel`

### 3.1 Hello World równoległy

```cpp
#include <iostream>
#include <omp.h>

int main() {
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        int total = omp_get_num_threads();
        std::cout << "Cześć od wątku " << tid << "/" << total << "\n";
    }
    // implicit barrier — main czeka aż wszystkie wątki skończą
}
```

Przykładowe wyjście (kolejność jest **niezdefiniowana!**):
```
Cześć od wątku 2/4
Cześć od wątku 0/4
Cześć od wątku 3/4
Cześć od wątku 1/4
```

### 3.2 Kontrola liczby wątków

```cpp
// Sposób 1: klauzula num_threads
#pragma omp parallel num_threads(8)
{ ... }

// Sposób 2: funkcja API
omp_set_num_threads(8);
#pragma omp parallel
{ ... }

// Sposób 3: zmienna środowiskowa (przed uruchomieniem)
// $ OMP_NUM_THREADS=8 ./program
```

Priorytet (od najwyższego):
```
1. klauzula num_threads(N)
2. omp_set_num_threads(N)
3. OMP_NUM_THREADS
4. domyślnie = liczba rdzeni logicznych
```

---

## 4. Klauzule danych

Klauzule danych decydują, które zmienne są **współdzielone** (shared) między wątkami,
a które są **prywatne** (private) — każdy wątek ma własną kopię.

### 4.1 Domyślna widoczność

```
Zmienne globalne             → shared (zawsze!)
Zmienne lokalne funkcji      → shared (jeśli wewnątrz parallel region)
Zmienne zadeklarowane WEWNĄTRZ bloku parallel → private (automatycznie)
```

```cpp
int x = 10;         // shared — wszystkie wątki widzą to samo x
#pragma omp parallel
{
    int y = 0;      // private — każdy wątek ma swoje y
    x = 42;         // ← WYŚCIG! (data race)
}
```

### 4.2 `shared(lista)`

```cpp
int suma = 0;
#pragma omp parallel shared(suma)
{
    // suma jest współdzielona — OSTROŻNIE, potrzebna synchronizacja!
}
```

### 4.3 `private(lista)`

```cpp
int tmp;
#pragma omp parallel private(tmp)
{
    tmp = omp_get_thread_num() * 2;  // każdy wątek ma własne tmp
    // tmp nie ma zdefiniowanej wartości początkowej!
}
// tmp po parallel jest NIEZDEFINIOWANE
```

### 4.4 `firstprivate(lista)` — kopia z wartością wejściową

```cpp
int base = 100;
#pragma omp parallel firstprivate(base)
{
    // Każdy wątek dostaje KOPIĘ base = 100
    base += omp_get_thread_num();  // bezpiecznie — prywatna kopia
    std::cout << base << "\n";     // np. 100, 101, 102, 103
}
// oryginalne base = 100 (niezmienione)
```

### 4.5 `lastprivate(lista)` — kopia z ostatniej iteracji

```cpp
int last_val;
#pragma omp parallel for lastprivate(last_val)
for (int i = 0; i < 10; ++i) {
    last_val = i * i;
}
// last_val = 81 (wartość z ostatniej iteracji i=9)
```

### 4.6 `default(none)` — najlepszy styl!

```cpp
int x = 5, y = 10;
int result = 0;

// Kompilator zmusi Cię do jawnej deklaracji KAŻDEJ zmiennej!
#pragma omp parallel default(none) shared(x, result) private(y)
{
    y = omp_get_thread_num();
    // x += y;  ← gdyby była taka linia to wyścig; z default(none) kompilator ostrzeże
}
```

> **Zalecenie**: Zawsze używaj `default(none)` — wymusza jawne myślenie o każdej zmiennej.

### Podsumowanie klauzul danych

| Klauzula | Inicjalizacja | Po parallel | Kiedy używać |
|----------|--------------|-------------|--------------|
| `shared(x)` | wspólna | zmieniona | duże tablice tylko do odczytu |
| `private(x)` | niezdef. | niezdef. | zmienne pomocnicze wewnątrz |
| `firstprivate(x)` | kopia | niezdef. | potrzebujesz wartości wejściowej |
| `lastprivate(x)` | niezdef. | z ostatniej iter. | potrzebujesz wyniku z końca |
| `default(none)` | — | — | jawność, bezpieczeństwo |

---

## 5. Work-sharing — podział pracy

Dyrektywy work-sharing **dzielą pracę** między wątki w regionie równoległym.
Nie tworzą nowych wątków — używają już istniejących.

### 5.1 `#pragma omp for` — równoległa pętla

```cpp
#include <vector>
#include <cmath>
#include <omp.h>

int main() {
    const int N = 1'000'000;
    std::vector<double> data(N, 1.0);

    #pragma omp parallel
    {
        #pragma omp for
        for (int i = 0; i < N; ++i) {
            data[i] = std::sqrt(data[i]) * 2.0;
        }
        // implicit barrier po pętli
    }
}
```

**Skrócona forma** (bardzo częsta):

```cpp
#pragma omp parallel for
for (int i = 0; i < N; ++i) {
    data[i] = std::sqrt(data[i]) * 2.0;
}
```

#### Wymagania dla `#pragma omp for`

Pętla musi mieć **canonicalną formę**, czyli:
```
for (init; cond; incr)
```
gdzie:
- `init`: inicjalizacja jednej zmiennej całkowitoliczbowej
- `cond`: porównanie z ustalonym kresem (np. `i < N`, `i <= N`)
- `incr`: prosta inkrementacja (np. `++i`, `i += 2`)
- **Brak** `break`, `continue`, `goto`, `return` wewnątrz (w pętli for)

```cpp
// ✅ Poprawne:
#pragma omp parallel for
for (int i = 0; i < N; ++i) { ... }

#pragma omp parallel for
for (int i = 0; i < N; i += 2) { ... }   // krok 2

// ❌ Niepoprawne:
#pragma omp parallel for
for (int i = 0; i < N; ++i) {
    if (warunek) break;  // BŁĄD!
}
```

### 5.2 `#pragma omp sections` — różne sekcje wykonywane przez różne wątki

```cpp
#pragma omp parallel sections
{
    #pragma omp section
    {
        // Ten blok może wykonać wątek 0
        std::cout << "Sekcja A: inicjalizacja danych\n";
        init_data();
    }

    #pragma omp section
    {
        // Ten blok może wykonać wątek 1
        std::cout << "Sekcja B: wczytanie pliku\n";
        load_file();
    }

    #pragma omp section
    {
        // Ten blok może wykonać wątek 2
        std::cout << "Sekcja C: inicjalizacja GUI\n";
        init_gui();
    }
}
// Wszystkie sekcje skończone — implicit barrier
```

> **Zastosowanie**: różne, niezależne zadania do wykonania jednocześnie.

### 5.3 `#pragma omp single` — jeden wątek, reszta czeka

```cpp
#pragma omp parallel
{
    // Wszystkie wątki robią to:
    do_parallel_work();

    #pragma omp single
    {
        // TYLKO JEDEN wątek wykona ten blok
        // Pozostałe czekają (implicit barrier po single)
        std::cout << "Inicjalizacja przez jeden wątek\n";
        global_init();
    }

    // Tutaj wszystkie wątki znowu działają
    do_more_work();
}
```

### 5.4 `#pragma omp master` — tylko wątek główny (id == 0)

```cpp
#pragma omp parallel
{
    do_parallel_work();

    #pragma omp master
    {
        // TYLKO wątek 0 (master), BEZ implicit barrier!
        // Pozostałe wątki NIE czekają — przechodzą dalej
        std::cout << "Master: postęp = " << progress << "\n";
    }

    // Tutaj wątki mogą być w różnych miejscach!
}
```

> **Różnica `single` vs `master`**:
> - `single`: dowolny wątek + implicit barrier po zakończeniu
> - `master`: zawsze wątek 0 + **brak** bariery

### 5.5 `nowait` — usunięcie niejawnej bariery

```cpp
#pragma omp parallel
{
    #pragma omp for nowait   // ← brak bariery po pętli!
    for (int i = 0; i < N; ++i) {
        compute_A(i);
    }
    // Wątek który skończył pętlę A może od razu zacząć pętlę B
    // (jeśli nie ma zależności między A i B)

    #pragma omp for
    for (int i = 0; i < N; ++i) {
        compute_B(i);
    }
}
```

---

## 6. Synchronizacja

### 6.1 `#pragma omp barrier` — jawna bariera

```cpp
#pragma omp parallel
{
    phase_one_work();           // wszystkie wątki robią fazę 1

    #pragma omp barrier         // wszyscy czekają tutaj

    phase_two_work();           // potem faza 2
}
```

### 6.2 `#pragma omp critical` — sekcja krytyczna

```cpp
int total = 0;

#pragma omp parallel
{
    int local = compute_local();

    #pragma omp critical   // tylko 1 wątek naraz
    {
        total += local;    // bezpieczna aktualizacja
    }
}
```

Możesz nazwać sekcje krytyczne (różne nazwy = niezależne locki!):

```cpp
#pragma omp critical(update_A)
{ A += val_a; }

#pragma omp critical(update_B)  // NIE blokuje się z update_A!
{ B += val_b; }
```

### 6.3 `#pragma omp atomic` — lekka operacja atomowa

```cpp
int counter = 0;

#pragma omp parallel for
for (int i = 0; i < N; ++i) {
    #pragma omp atomic
    counter++;   // atomowe inkrementowanie — szybsze niż critical!
}
```

Obsługiwane operacje atomowe:

```cpp
#pragma omp atomic update    // x++, x--, x += expr, x -= expr, x *= expr itp.
#pragma omp atomic read      // v = x;
#pragma omp atomic write     // x = expr;
#pragma omp atomic capture   // v = x; x += expr;  (łączone read+update)
```

#### `critical` vs `atomic`

| | `critical` | `atomic` |
|---|---|---|
| Zakres | dowolny blok kodu | jedna prosta operacja |
| Wydajność | wolniejszy (mutex) | szybszy (sprzętowe instrukcje) |
| Elastyczność | pełna | ograniczona do prostych op. |
| Kiedy użyć | skomplikowana logika | inkrementacja, prosta arytmetyka |

### 6.4 `#pragma omp ordered` — zachowanie kolejności iteracji

```cpp
#pragma omp parallel for ordered
for (int i = 0; i < N; ++i) {
    double result = heavy_compute(i);  // obliczenia równolegle

    #pragma omp ordered
    {
        // Ten blok jest wykonywany w KOLEJNOŚCI iteracji (0, 1, 2, ...)
        output_file << result << "\n";
    }
}
```

### 6.5 `omp_lock_t` — manualne locki

```cpp
#include <omp.h>

omp_lock_t lock;
omp_init_lock(&lock);

#pragma omp parallel
{
    omp_set_lock(&lock);
    // sekcja krytyczna
    omp_unset_lock(&lock);
}

omp_destroy_lock(&lock);
```

```cpp
// Wersja RAII — warto owinąć locka
struct OmpLockGuard {
    omp_lock_t& lock_;
    explicit OmpLockGuard(omp_lock_t& l) : lock_(l) { omp_set_lock(&lock_); }
    ~OmpLockGuard() { omp_unset_lock(&lock_); }
};
```

---

## 7. Zarządzanie wątkami — API OpenMP

### 7.1 Podstawowe funkcje

```cpp
#include <omp.h>

int tid   = omp_get_thread_num();     // id bieżącego wątku (0..N-1)
int nthr  = omp_get_num_threads();    // liczba wątków w bieżącym regionie
int max_t = omp_get_max_threads();    // maksymalna możliwa liczba wątków
int ncpus = omp_get_num_procs();      // liczba procesorów/rdzeni
bool inpar = omp_in_parallel();       // czy jesteśmy w regionie równoległym?

omp_set_num_threads(4);               // ustaw domyślną liczbę wątków
omp_set_dynamic(1);                   // pozwól bibliotece dostosować liczbę wątków
omp_set_nested(1);                    // włącz zagnieżdżoną równoległość (deprecated 5.0)
```

### 7.2 Pomiar czasu (high-resolution)

```cpp
double t_start = omp_get_wtime();   // czas w sekundach (wall-clock)

#pragma omp parallel for
for (int i = 0; i < N; ++i)
    heavy_work(i);

double t_end = omp_get_wtime();
std::cout << "Czas: " << (t_end - t_start) << " s\n";
```

### 7.3 Zmienne środowiskowe OpenMP

| Zmienna | Opis | Przykład |
|---------|------|---------|
| `OMP_NUM_THREADS` | liczba wątków | `OMP_NUM_THREADS=8` |
| `OMP_SCHEDULE` | strategia schedulera | `OMP_SCHEDULE="dynamic,4"` |
| `OMP_DYNAMIC` | dynamiczna liczba wątków | `OMP_DYNAMIC=true` |
| `OMP_NESTED` | zagnieżdżona równoległość | `OMP_NESTED=true` |
| `OMP_PROC_BIND` | przypinanie do rdzeni | `OMP_PROC_BIND=close` |
| `OMP_PLACES` | definicja miejsc (place) | `OMP_PLACES=cores` |
| `OMP_WAIT_POLICY` | czekanie wątków | `OMP_WAIT_POLICY=passive` |
| `OMP_STACKSIZE` | rozmiar stosu wątku | `OMP_STACKSIZE=8M` |

---

## 8. Klauzule `schedule` — strategie podziału iteracji

Klauzula `schedule` decyduje, **jak iteracje pętli `for` są rozdzielane między wątki**.

```cpp
#pragma omp parallel for schedule(typ, chunk_size)
for (int i = 0; i < N; ++i) { ... }
```

### 8.1 `static`

```
Iteracje dzielone na bloki o stałym rozmiarze, przydzielane wątkom z góry (round-robin).
```

```cpp
// N=12, 3 wątki, chunk=4:
// Wątek 0: i = 0,1,2,3
// Wątek 1: i = 4,5,6,7
// Wątek 2: i = 8,9,10,11

#pragma omp parallel for schedule(static, 4)
for (int i = 0; i < 12; ++i) { work(i); }

// Bez chunk → każdy wątek dostaje N/num_threads iteracji (jeden blok):
// Wątek 0: i = 0,1,2,3   Wątek 1: i = 4,5,6,7   Wątek 2: i = 8,9,10,11
#pragma omp parallel for schedule(static)
for (int i = 0; i < 12; ++i) { work(i); }
```

**Zastosowanie**: regularne iteracje (każda ma podobny czas wykonania).
Minimalne narzuty schedulera.

### 8.2 `dynamic`

```
Wątki pobierają iteracje dynamicznie z puli — gdy skończą swój chunk, biorą następny.
```

```cpp
// N=12, 3 wątki, chunk=2:
// Wątek 0: bierze [0,1], potem [6,7], potem [10,11]
// Wątek 1: bierze [2,3], potem [8,9]
// Wątek 2: bierze [4,5], potem [12...] — brak, koniec
// (kolejność jest dynamiczna — zależy od szybkości wątków)

#pragma omp parallel for schedule(dynamic, 2)
for (int i = 0; i < 12; ++i) { irregular_work(i); }
```

**Zastosowanie**: nieregularne iteracje (różny czas wykonania). Lepsze balansowanie,
ale wyższy narzut synchronizacji.

### 8.3 `guided`

```
Jak dynamic, ale rozmiar bloku maleje — na początku duże bloki (dla efektywności),
pod koniec małe bloki (dla równomiernego zakończenia).
```

```cpp
#pragma omp parallel for schedule(guided, 2)
// chunk=2 to minimalna wartość bloku
for (int i = 0; i < N; ++i) { work(i); }
```

### 8.4 `auto` i `runtime`

```cpp
// auto: kompilator lub biblioteka sama decyduje
#pragma omp parallel for schedule(auto)
for (int i = 0; i < N; ++i) { work(i); }

// runtime: decyzja podejmowana w czasie wykonania na podstawie OMP_SCHEDULE
#pragma omp parallel for schedule(runtime)
for (int i = 0; i < N; ++i) { work(i); }
// $ OMP_SCHEDULE="dynamic,4" ./program
```

### Kiedy używać którego?

```
Czas iteracji:   STAŁY          → static   (zero overhead)
Czas iteracji:   ZMIENNY        → dynamic  (lepszy balans)
Iteracje na końcu trudniejsze → guided
Testowanie wydajności           → runtime  (zmień bez recompilacji)
```

---

## 9. Reduction — redukcja równoległa

### 9.1 Podstawowy przykład — suma

```cpp
double sum = 0.0;

#pragma omp parallel for reduction(+:sum)
for (int i = 0; i < N; ++i) {
    sum += data[i];   // każdy wątek ma prywatne sum; na końcu łączone operatorem +
}
// sum zawiera pełną sumę wszystkich elementów
```

**Co się dzieje wewnętrznie?**:

```
1. Każdy wątek dostaje PRYWATNĄ kopię sum, zainicjalizowaną na 0 (element neutralny dla +)
2. Każdy wątek aktualizuje swoją kopię niezależnie
3. Po pętli: wszystkie kopie są łączone (suma) do zmiennej sum w głównym wątku
```

### 9.2 Dostępne operatory redukcji

| Operator | Inicjalizacja |
|----------|--------------|
| `+` | `0` |
| `-` | `0` |
| `*` | `1` |
| `&` | `~0` (same bity 1) |
| `|` | `0` |
| `^` | `0` |
| `&&` | `1` |
| `\|\|` | `0` |
| `max` | `-∞` |
| `min` | `+∞` |

```cpp
int max_val = INT_MIN;
#pragma omp parallel for reduction(max:max_val)
for (int i = 0; i < N; ++i) {
    if (data[i] > max_val) max_val = data[i];
}

bool any_negative = false;
#pragma omp parallel for reduction(||:any_negative)
for (int i = 0; i < N; ++i) {
    if (data[i] < 0) any_negative = true;
}
```

### 9.3 Własny operator redukcji (OpenMP 4.0+)

```cpp
struct Point { double x, y; };

// Deklaracja własnej operacji redukcji
#pragma omp declare reduction(add_points : Point : omp_out.x += omp_in.x, omp_out.y += omp_in.y) \
    initializer(omp_priv = Point{0.0, 0.0})

Point center{0.0, 0.0};
#pragma omp parallel for reduction(add_points:center)
for (int i = 0; i < N; ++i) {
    center.x += points[i].x;
    center.y += points[i].y;
}
center.x /= N;
center.y /= N;
```

---

## 10. Zaawansowane OpenMP — Tasks

### 10.1 Po co Tasks?

`#pragma omp for` działa świetnie dla **regularnych pętli**. Co z rekursją,
drzewami, listami, algorytmami divide-and-conquer?

```
Problem: QuickSort → podział nie jest równomierny z góry!

Tradycyjne pętle OpenMP nie nadają się do:
  - rekurencji
  - przetwarzania list/drzew
  - algorytmów "work-stealing"
  - zadań o nieznanej z góry liczbie
```

### 10.2 `#pragma omp task`

```cpp
#pragma omp parallel
{
    #pragma omp single
    {
        // Jeden wątek tworzy zadania (taski)
        for (int i = 0; i < N; ++i) {
            #pragma omp task
            {
                // To jest zadanie — może być wykonane przez DOWOLNY wątek
                process_item(i);
            }
        }
        // Na końcu single — implicit taskwait (czeka na swoje dzieci)
    }
}
```

### 10.3 `taskwait` — czekaj na zakończenie zadań potomnych

```cpp
void process_tree(Node* node) {
    if (!node) return;

    #pragma omp task
    process_tree(node->left);   // lewe poddrzewo jako task

    #pragma omp task
    process_tree(node->right);  // prawe poddrzewo jako task

    #pragma omp taskwait        // czekaj aż oba poddrzewa skończą
    merge_results(node);        // dopiero teraz łącz wyniki
}

// Uruchomienie:
#pragma omp parallel
{
    #pragma omp single
    process_tree(root);
}
```

### 10.4 Równoległy QuickSort z Tasks

```cpp
#include <algorithm>
#include <omp.h>

void parallel_quicksort(int* arr, int left, int right) {
    if (right - left < 1000) {
        // Dla małych zakresów — szeregowo (brak sensu tworzyć tasków)
        std::sort(arr + left, arr + right + 1);
        return;
    }

    // Podział (partition)
    int pivot = arr[(left + right) / 2];
    int i = left, j = right;
    while (i <= j) {
        while (arr[i] < pivot) ++i;
        while (arr[j] > pivot) --j;
        if (i <= j) { std::swap(arr[i], arr[j]); ++i; --j; }
    }

    #pragma omp task
    parallel_quicksort(arr, left, j);    // sortuj lewą część

    #pragma omp task
    parallel_quicksort(arr, i, right);   // sortuj prawą część

    #pragma omp taskwait
}

int main() {
    int arr[] = {5, 3, 8, 1, 9, 2, 7, 4, 6};
    int n = 9;

    #pragma omp parallel
    {
        #pragma omp single
        parallel_quicksort(arr, 0, n - 1);
    }
}
```

### 10.5 Klauzule tasków

```cpp
// shared/private w taskach
int result;
#pragma omp task shared(result)
{ result = compute(); }   // result jest współdzielony ze środowiskiem tworzącym task

// firstprivate — kopia wartości w momencie tworzenia taska
int x = counter++;
#pragma omp task firstprivate(x)
{ use(x); }    // x w tasku = wartość w momencie tworzenia, niezależnie od późniejszych zmian

// depend — zależności między taskami
int A, B, C;
#pragma omp task depend(out:A)
{ A = compute_A(); }

#pragma omp task depend(out:B)
{ B = compute_B(); }

#pragma omp task depend(in:A,B) depend(out:C)
{ C = combine(A, B); }   // czeka aż A i B będą gotowe

// priority — priorytet zadania (wyższy = preferowany)
#pragma omp task priority(10)
{ critical_work(); }
```

### 10.6 `taskgroup` — bariera dla wszystkich potomnych tasków

```cpp
#pragma omp taskgroup
{
    // Wszystkie taski stworzone w tym bloku muszą się zakończyć
    // zanim wyjdziemy z taskgroup
    for (int i = 0; i < N; ++i) {
        #pragma omp task
        { work(i); }
    }
}
// Tutaj wszystkie taski z taskgroup są zakończone
```

---

## 11. SIMD — wektoryzacja

### 11.1 `#pragma omp simd`

```cpp
// Hint dla kompilatora: wektoryzuj tę pętlę (użyj SSE/AVX/NEON)
#pragma omp simd
for (int i = 0; i < N; ++i) {
    c[i] = a[i] + b[i];   // 256-bit AVX → 8 dodawań float na raz
}
```

### 11.2 `parallel for simd` — równoległość + SIMD jednocześnie

```cpp
#pragma omp parallel for simd
for (int i = 0; i < N; ++i) {
    result[i] = std::sin(x[i]) + std::cos(y[i]);
}
// Iteracje podzielone między wątki, każdy wątek wektoryzuje swój kawałek
```

### 11.3 Klauzule SIMD

```cpp
// safelen — maksymalny bezpieczny wektor (jeśli są zależności co safelen iteracji)
#pragma omp simd safelen(4)
for (int i = 4; i < N; ++i) {
    a[i] = a[i-4] + b[i];  // zależność co 4 — bezpieczny wektor 4
}

// linear — zmienna rośnie liniowo
int ptr = 0;
#pragma omp simd linear(ptr:1)
for (int i = 0; i < N; ++i) {
    c[ptr++] = a[i] + b[i];
}

// reduction w simd
double sum = 0.0;
#pragma omp simd reduction(+:sum)
for (int i = 0; i < N; ++i) {
    sum += a[i] * b[i];   // iloczyn skalarny wektoryzowany
}
```

---

## 12. Zagnieżdżona równoległość

### 12.1 Podstawowy przykład

```cpp
omp_set_nested(1);   // włącz zagnieżdżoną równoległość (lub OMP_NESTED=true)

#pragma omp parallel num_threads(2)   // poziom 1: 2 wątki
{
    int outer = omp_get_thread_num();

    #pragma omp parallel num_threads(3)   // poziom 2: 3 wątki na każdy z 2 = 6 wątków total
    {
        int inner = omp_get_thread_num();
        std::cout << "Outer=" << outer << " Inner=" << inner << "\n";
    }
}
```

### 12.2 Kiedy stosować zagnieżdżenie?

```
✅ Dobre zastosowanie:
  - Algorytmy dwupoziomowe (np. matrix-matrix multiplication: wiersze/kolumny)
  - Systemy NUMA — zewnętrzny poziom = gniazda, wewnętrzny = rdzenie

❌ Złe zastosowanie:
  - Gdy łączna liczba wątków przekracza liczbę rdzeni
  - Gdy nie ma wystarczająco dużo pracy na każdym poziomie
```

### 12.3 `omp_get_level()` — poziom zagnieżdżenia

```cpp
#pragma omp parallel
{
    int level = omp_get_level();       // 1 dla zewnętrznego
    int ancestor = omp_get_ancestor_thread_num(1);  // id wątku w poziomie 1

    #pragma omp parallel
    {
        level = omp_get_level();       // 2 dla wewnętrznego
    }
}
```

---

## 13. Model pamięci OpenMP

### 13.1 Flush — synchronizacja pamięci

OpenMP używa **relaxed memory model** — wątki mogą trzymać zmienne w rejestrach/cache.
`flush` wymusza synchronizację z pamięcią główną.

```cpp
int ready = 0;
int data  = 0;

// Producent:
data = 42;
#pragma omp flush(data)   // upewnij się, że data jest widoczna
ready = 1;
#pragma omp flush(ready)  // upewnij się, że ready jest widoczne

// Konsument:
int r = 0;
while (!r) {
    #pragma omp flush(ready)
    r = ready;
}
#pragma omp flush(data)
use(data);   // bezpieczne — data jest zsynchronizowane
```

> **Uwaga**: `#pragma omp critical`, `barrier`, `atomic` **automatycznie wykonują flush**
> — nie trzeba go dodawać ręcznie przy tych konstrukcjach.

### 13.2 `threadprivate` — globalne prywatne zmienne

```cpp
int counter = 0;
#pragma omp threadprivate(counter)
// Teraz counter jest GLOBALNY, ale każdy wątek ma WŁASNĄ KOPIĘ

void increment() {
    counter++;   // każdy wątek inkrementuje swój counter
}

#pragma omp parallel num_threads(4) copyin(counter)
// copyin: inicjalizuje prywatne kopie wartością z głównego wątku
{
    increment();
    std::cout << counter << "\n";   // 1 (każdy wątek osobno)
}
```

---

## 14. Offloading na GPU / akcelerator (`target`)

### 14.1 Podstawowy `target` (OpenMP 4.0+)

```cpp
int N = 1000;
std::vector<int> a(N, 1), b(N, 2), c(N);

#pragma omp target                    // wykonaj na GPU/akceleratorze
#pragma omp parallel for              // równolegle na GPU
for (int i = 0; i < N; ++i) {
    c[i] = a[i] + b[i];
}
// c jest automatycznie kopiowane z powrotem do CPU
```

### 14.2 Zarządzanie danymi `map`

```cpp
#pragma omp target map(to: a[0:N], b[0:N]) map(from: c[0:N])
// to:   kopiuj do GPU przed kernelem
// from: kopiuj z GPU po kernelu
// tofrom: w obie strony (domyślne dla istniejących zmiennych)
{
    #pragma omp parallel for
    for (int i = 0; i < N; ++i) {
        c[i] = a[i] * b[i];
    }
}
```

### 14.3 `teams` i `distribute`

```cpp
// Bardziej efektywny model GPU: teams (grupy wątków = bloki CUDA)
#pragma omp target teams distribute parallel for
for (int i = 0; i < N; ++i) {
    c[i] = a[i] + b[i];
}
```

---

## 15. Pułapki i dobre praktyki

### 15.1 ❌ Wyścig danych (Data Race)

```cpp
// ❌ WYŚCIG! Wiele wątków zapisuje do sum jednocześnie
int sum = 0;
#pragma omp parallel for
for (int i = 0; i < N; ++i) {
    sum += data[i];   // nie atomowe!
}

// ✅ POPRAWNIE: reduction
#pragma omp parallel for reduction(+:sum)
for (int i = 0; i < N; ++i) {
    sum += data[i];
}

// ✅ POPRAWNIE: atomic
#pragma omp parallel for
for (int i = 0; i < N; ++i) {
    #pragma omp atomic
    sum += data[i];
}
```

### 15.2 ❌ False sharing — ukryta degradacja wydajności

```cpp
// ❌ Zła wersja: tablica lokalna wątków — indeksy mogą być na tej samej linii cache!
int partial_sum[4];   // 4 wątki, 4 elementy, jeden int = 4B
// Linia cache = 64B → wszystkie 4 inty mieszczą się w JEDNEJ linii!
// Wątek 0 pisze partial_sum[0] → unieważnia linię → wątek 1 musi ją wczytać od nowa

#pragma omp parallel num_threads(4)
{
    int tid = omp_get_thread_num();
    for (int i = tid; i < N; i += 4)
        partial_sum[tid] += data[i];   // FALSE SHARING!
}

// ✅ Dobra wersja: padding / użyj reduction
struct alignas(64) PaddedInt { int val; };  // 64 = rozmiar linii cache
PaddedInt partial_sum[4];

// albo po prostu:
#pragma omp parallel for reduction(+:sum)
for (int i = 0; i < N; ++i) sum += data[i];
```

### 15.3 ❌ Błędy z `private` — niezainicjalizowana zmienna

```cpp
int x = 100;
#pragma omp parallel private(x)
{
    // ❌ x jest NIEZAINICJALIZOWANE — undefined behavior!
    std::cout << x << "\n";   // może wypisać śmieć

    // ✅ Użyj firstprivate jeśli potrzebujesz wartości wejściowej
}
```

### 15.4 ❌ Pętla nie-kanoniczna

```cpp
// ❌ Nie można zrównoleglić — brak kanonicznej formy
std::list<int> lst = {1, 2, 3, 4, 5};
#pragma omp parallel for
for (auto it = lst.begin(); it != lst.end(); ++it) {   // BŁĄD!
    process(*it);
}

// ✅ Dla nie-kanonicznej pętli — użyj tasków
#pragma omp parallel
{
    #pragma omp single
    for (auto& item : lst) {
        #pragma omp task firstprivate(item)
        process(item);
    }
}
```

### 15.5 ❌ Zbyt drobnoziarnista równoległość (overhead > zysk)

```cpp
// ❌ Narzut tworzenia wątków > korzyść z równoległości
#pragma omp parallel for
for (int i = 0; i < 10; ++i) {   // za mało pracy!
    result[i] = i * 2;
}

// Zasada kciuka: równoległość opłaca się gdy czas_pracy >> czas_synchronizacji
// Dla OpenMP: zazwyczaj sensowne powyżej ~1000 iteracji (zależy od pracy)
```

### 15.6 Checklista poprawnego kodu OpenMP

```
□ Używam default(none) i jawnie deklaruję każdą zmienną
□ Nie ma zmiennych shared, do których wiele wątków pisze bez synchronizacji
□ Używam reduction zamiast critical/atomic dla agregacji
□ Sprawdziłem czy pętla ma kanoniczną formę
□ Poza regionem równoległym nie zakładam kolejności wykonania wątków
□ Pomierzyłem czas — upewniłem się że jest faktyczne przyspieszenie
□ Skompilowałem z sanitizers (ThreadSanitizer) w wersji debug
```

---

## 16. Porównanie: OpenMP vs `std::thread` vs TBB

| Kryterium | OpenMP | `std::thread` / `std::jthread` | Intel TBB |
|-----------|--------|-------------------------------|-----------|
| Łatwość użycia | ⭐⭐⭐⭐⭐ | ⭐⭐⭐ | ⭐⭐⭐⭐ |
| Kontrola | ⭐⭐⭐ | ⭐⭐⭐⭐⭐ | ⭐⭐⭐⭐ |
| Przenośność | ⭐⭐⭐⭐ | ⭐⭐⭐⭐⭐ | ⭐⭐⭐⭐ |
| HPC / obliczenia naukowe | ⭐⭐⭐⭐⭐ | ⭐⭐ | ⭐⭐⭐⭐ |
| GPU offloading | ⭐⭐⭐⭐ | ❌ | ⭐⭐⭐ |
| Incremental parallelism | ⭐⭐⭐⭐⭐ | ❌ | ⭐⭐ |
| Work-stealing scheduler | ❌ / task | ❌ | ⭐⭐⭐⭐⭐ |

---

## Ćwiczenia

### Ćwiczenie 1 — Suma równoległa

Napisz program, który oblicza sumę elementów tablicy o rozmiarze 10 000 000
używając `reduction`. Zmierz czas dla 1, 2, 4 i 8 wątków.

```cpp
#include <vector>
#include <numeric>
#include <iostream>
#include <omp.h>

int main() {
    const int N = 10'000'000;
    std::vector<double> data(N);
    std::iota(data.begin(), data.end(), 0.0);

    for (int t : {1, 2, 4, 8}) {
        omp_set_num_threads(t);
        double sum = 0.0;
        double t0 = omp_get_wtime();

        #pragma omp parallel for reduction(+:sum)
        for (int i = 0; i < N; ++i) sum += data[i];

        double t1 = omp_get_wtime();
        std::cout << "Wątki=" << t << " suma=" << sum
                  << " czas=" << (t1-t0) << "s\n";
    }
}
```

### Ćwiczenie 2 — Mnożenie macierzy

```cpp
#include <vector>
#include <omp.h>

void matmul(const std::vector<std::vector<double>>& A,
            const std::vector<std::vector<double>>& B,
            std::vector<std::vector<double>>& C,
            int N) {
    #pragma omp parallel for collapse(2) schedule(static)
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            double sum = 0.0;
            for (int k = 0; k < N; ++k)
                sum += A[i][k] * B[k][j];
            C[i][j] = sum;
        }
    }
}
```

> **`collapse(2)`**: łączy dwie zewnętrzne pętle w jedną przestrzeń iteracji
> → lepsze balansowanie (N² iteracji zamiast N).

### Ćwiczenie 3 — Równoległe sortowanie (merge sort)

```cpp
#include <algorithm>
#include <vector>
#include <omp.h>

void parallel_mergesort(std::vector<int>& arr, int l, int r, int depth = 0) {
    if (r - l <= 1000 || depth > 4) {
        std::sort(arr.begin() + l, arr.begin() + r);
        return;
    }
    int mid = l + (r - l) / 2;

    #pragma omp task shared(arr) if(depth < 3)
    parallel_mergesort(arr, l, mid, depth + 1);

    #pragma omp task shared(arr) if(depth < 3)
    parallel_mergesort(arr, mid, r, depth + 1);

    #pragma omp taskwait
    std::inplace_merge(arr.begin() + l, arr.begin() + mid, arr.begin() + r);
}

int main() {
    std::vector<int> data = {5, 2, 8, 1, 9, 3, 7, 4, 6};
    #pragma omp parallel
    {
        #pragma omp single
        parallel_mergesort(data, 0, data.size());
    }
}
```

### Ćwiczenie 4 — Pipeline z sekcjami

```cpp
// Równoległy pipeline: wczytaj dane → przetworz → zapisz
// (3 niezależne etapy, każdy może działać na innym fragmencie jednocześnie)
#include <queue>
#include <omp.h>

void pipeline_example() {
    const int CHUNKS = 10;

    #pragma omp parallel sections num_threads(3)
    {
        #pragma omp section
        {
            for (int c = 0; c < CHUNKS; ++c) load_chunk(c);
        }

        #pragma omp section
        {
            for (int c = 0; c < CHUNKS; ++c) process_chunk(c);
        }

        #pragma omp section
        {
            for (int c = 0; c < CHUNKS; ++c) save_chunk(c);
        }
    }
}
```

---

## Szybka ściągawka — składnia

```cpp
// ── TWORZENIE REGIONU ───────────────────────────────────────────────
#pragma omp parallel [klauzule]
{ ... }

// ── WORK-SHARING ─────────────────────────────────────────────────────
#pragma omp for [schedule(typ,chunk)] [collapse(N)] [nowait] [reduction]
#pragma omp sections [nowait]
  #pragma omp section
#pragma omp single [nowait]
#pragma omp master

// ── SKRÓTY ──────────────────────────────────────────────────────────
#pragma omp parallel for [klauzule]
#pragma omp parallel sections [klauzule]

// ── SYNCHRONIZACJA ──────────────────────────────────────────────────
#pragma omp barrier
#pragma omp critical [(nazwa)]
#pragma omp atomic [read|write|update|capture]
#pragma omp ordered
#pragma omp flush [(lista)]

// ── TASKS ────────────────────────────────────────────────────────────
#pragma omp task [shared|private|firstprivate|depend|priority|if]
#pragma omp taskwait
#pragma omp taskgroup

// ── SIMD ─────────────────────────────────────────────────────────────
#pragma omp simd [safelen|linear|reduction]
#pragma omp parallel for simd

// ── KLAUZULE DANYCH ──────────────────────────────────────────────────
shared(lista)         private(lista)       firstprivate(lista)
lastprivate(lista)    reduction(op:lista)  default(none|shared|private)
copyin(lista)

// ── KLAUZULE HARMONOGRAMOWANIA ───────────────────────────────────────
schedule(static [,chunk])
schedule(dynamic [,chunk])
schedule(guided [,chunk])
schedule(auto)
schedule(runtime)

// ── KLAUZULA COLLAPSE ────────────────────────────────────────────────
collapse(N)    // łączy N poziomów pętli for w jedną przestrzeń iteracji

// ── API ──────────────────────────────────────────────────────────────
omp_get_thread_num()      omp_get_num_threads()    omp_get_max_threads()
omp_get_num_procs()       omp_in_parallel()        omp_get_wtime()
omp_set_num_threads(N)    omp_set_dynamic(bool)    omp_set_nested(bool)
omp_init_lock(&lock)      omp_set_lock(&lock)      omp_unset_lock(&lock)
omp_destroy_lock(&lock)
```

---

## Podsumowanie

```
OpenMP to potężne, ale proste w użyciu API do programowania równoległego.

PODSTAWY:
  #pragma omp parallel        → uruchom region równoległy
  #pragma omp parallel for    → zrównolegnij pętlę
  reduction(+:sum)            → bezpieczna agregacja
  default(none)               → wymuszaj jawność zmiennych

SYNCHRONIZACJA:
  critical  → dowolna sekcja krytyczna (mutex)
  atomic    → prosta operacja (szybszy niż critical)
  barrier   → wszyscy czekają
  ordered   → zachowaj kolejność iteracji

ZAAWANSOWANE:
  task/taskwait   → dynamiczne zadania (rekurencja, drzewa)
  schedule(...)   → dopasuj podział do kształtu obciążenia
  simd            → wektoryzacja pętli
  collapse(N)     → łącz zagnieżdżone pętle
  target          → offloading na GPU

PUŁAPKI:
  ✗ Shared mutable state bez synchronizacji → data race
  ✗ False sharing → paduj tablice do 64B
  ✗ Zbyt drobnoziarnista praca → overhead > zysk
  ✗ private() bez firstprivate() gdy potrzebujesz wartości
```
