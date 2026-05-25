# MPI w C++ — od podstaw do zaawansowanych technik
## Notatki do wykładu/ćwiczeń z programowania współbieżnego

---

## Spis treści

1. [Czym jest MPI?](#1-czym-jest-mpi)
2. [Model programowania i kompilacja](#2-model-programowania-i-kompilacja)
3. [Podstawowe operacje — Hello World](#3-podstawowe-operacje--hello-world)
4. [Komunikacja punkt-punkt (point-to-point)](#4-komunikacja-punkt-punkt-point-to-point)
5. [Komunikacja nieblokująca](#5-komunikacja-nieblokująca)
6. [Operacje kolektywne (collective)](#6-operacje-kolektywne-collective)
7. [Redukcja i operacje globalne](#7-redukcja-i-operacje-globalne)
8. [Typy danych MPI i typy pochodne](#8-typy-danych-mpi-i-typy-pochodne)
9. [Komunikatory i grupy procesów](#9-komunikatory-i-grupy-procesów)
10. [Topologie wirtualne](#10-topologie-wirtualne)
11. [Komunikacja jednostronna (RMA / One-sided)](#11-komunikacja-jednostronna-rma--one-sided)
12. [Równoległe wejście/wyjście (MPI I/O)](#12-równoległe-wejściewyjście-mpi-io)
13. [Pułapki i dobre praktyki](#13-pułapki-i-dobre-praktyki)
14. [Wzorce projektowe i zadania](#14-wzorce-projektowe-i-zadania)
15. [Ściąga — najważniejsze funkcje](#15-ściąga--najważniejsze-funkcje)

---

## 1. Czym jest MPI?

**MPI** (Message Passing Interface) to **standard komunikacji między procesami**
w systemach z pamięcią rozproszoną. Procesy nie współdzielą pamięci — komunikują
się wyłącznie przez **przesyłanie wiadomości** (message passing).

### Kluczowe cechy

| Cecha | Opis |
|-------|------|
| **Pamięć rozproszona** | Każdy proces ma własną, prywatną pamięć |
| **Jawna komunikacja** | Programista decyduje co, kiedy i komu wysłać |
| **Skalowalność** | Działa na 1 do setek tysięcy procesów |
| **Przenośność** | C, C++, Fortran; klastry, superkomputery, laptopy |
| **SPMD** | Single Program, Multiple Data — ten sam kod, różne dane |

### Model komunikacji

```
Proces 0             Proces 1             Proces 2
┌──────────┐        ┌──────────┐        ┌──────────┐
│  Pamięć  │        │  Pamięć  │        │  Pamięć  │
│  lokalna │        │  lokalna │        │  lokalna │
└────┬─────┘        └────┬─────┘        └────┬─────┘
     │                    │                    │
     │◄───── Send ───────►│◄───── Send ───────►│
     │                    │                    │
═════╧════════════════════╧════════════════════╧═════
              SIEĆ / INTERCONNECT
```

### Porównanie: OpenMP vs MPI

| | OpenMP | MPI |
|---|--------|-----|
| Pamięć | Współdzielona | Rozproszona |
| Komunikacja | Przez zmienne | Przez wiadomości |
| Granulacja | Wątki (fine-grained) | Procesy (coarse-grained) |
| Skala | 1 węzeł | Tysiące węzłów |
| Łatwość | Prosta (#pragma) | Wymaga jawnej komunikacji |
| Debugowanie | Data race | Deadlock, buffer overflow |

---

## 2. Model programowania i kompilacja

### 2.1 Instalacja

```bash
# Ubuntu/Debian
sudo apt install libopenmpi-dev openmpi-bin

# Fedora/RHEL
sudo dnf install openmpi-devel

# macOS (Homebrew)
brew install open-mpi
```

### 2.2 Kompilacja

```bash
# Kompilacja z wrapperem MPI (zalecane!)
mpicxx -O2 program.cpp -o program

# Equivalent ręczny (nie zalecany)
g++ -O2 program.cpp -o program -lmpi

# Sprawdzenie co robi wrapper
mpicxx --showme
# → g++ ... -I/usr/include/openmpi -L/usr/lib/openmpi -lmpi_cxx -lmpi ...
```

### 2.3 Uruchomienie

```bash
# Uruchomienie z 4 procesami
mpirun -np 4 ./program

# Alternatywnie
mpiexec -n 4 ./program

# Na klastrze z plikiem hostów
mpirun -np 16 --hostfile hosts.txt ./program
```

> **Każdy proces** dostaje własną kopię programu, własny stos, własną pamięć.
> Procesy mogą (ale nie muszą) działać na różnych maszynach!

### 2.4 Niezbędne nagłówki

```cpp
#include <mpi.h>    // wszystkie funkcje MPI
#include <iostream>
```

---

## 3. Podstawowe operacje — Hello World

### 3.1 Minimalny program MPI

```cpp
#include <mpi.h>
#include <iostream>

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);  // ← Inicjalizacja MPI (OBOWIĄZKOWA)

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);  // Mój numer (0..size-1)
    MPI_Comm_size(MPI_COMM_WORLD, &size);  // Łączna liczba procesów

    std::cout << "Cześć od procesu " << rank << "/" << size << "\n";

    MPI_Finalize();  // ← Zakończenie MPI (OBOWIĄZKOWE)
    return 0;
}
```

```bash
$ mpicxx hello.cpp -o hello
$ mpirun -np 4 ./hello
Cześć od procesu 2/4
Cześć od procesu 0/4
Cześć od procesu 3/4
Cześć od procesu 1/4
```

> Kolejność wyjścia jest **niezdefiniowana** — procesy działają niezależnie!

### 3.2 Anatomia programu MPI

```
MPI_Init()              ← Wejście do świata MPI
│
├── MPI_Comm_rank()     ← Kim jestem?
├── MPI_Comm_size()     ← Ilu nas jest?
│
├── ... obliczenia ...
├── ... komunikacja ...
│
MPI_Finalize()          ← Wyjście ze świata MPI
```

### 3.3 Zasady `MPI_Init` / `MPI_Finalize`

```cpp
// ✗ BŁĄD: Wywołanie MPI przed Init
MPI_Comm_rank(MPI_COMM_WORLD, &rank);  // undefined behavior!
MPI_Init(&argc, &argv);

// ✗ BŁĄD: Wywołanie MPI po Finalize
MPI_Finalize();
MPI_Send(...);  // undefined behavior!

// ✗ BŁĄD: Brak Finalize — zasoby mogą nie zostać zwolnione

// ✓ POPRAWNE: Init jako pierwsza, Finalize jako ostatnia operacja MPI
```

---

## 4. Komunikacja punkt-punkt (point-to-point)

### 4.1 `MPI_Send` / `MPI_Recv` — podstawy

```cpp
// Wysyłanie:
int MPI_Send(
    const void* buf,      // wskaźnik na dane do wysłania
    int count,            // ile elementów
    MPI_Datatype type,    // typ danych (MPI_INT, MPI_DOUBLE, ...)
    int dest,             // rank odbiorcy
    int tag,              // etykieta wiadomości (dowolna liczba)
    MPI_Comm comm         // komunikator (MPI_COMM_WORLD)
);

// Odbieranie:
int MPI_Recv(
    void* buf,            // bufor na odebrane dane
    int count,            // max elementów do odebrania
    MPI_Datatype type,    // typ danych
    int source,           // rank nadawcy (lub MPI_ANY_SOURCE)
    int tag,              // etykieta (lub MPI_ANY_TAG)
    MPI_Comm comm,        // komunikator
    MPI_Status* status    // informacje o odebranej wiadomości
);
```

### 4.2 Ping-Pong — klasyczny przykład

```cpp
#include <mpi.h>
#include <iostream>

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int ping_pong_count = 0;
    int partner = (rank == 0) ? 1 : 0;

    for (int i = 0; i < 10; ++i) {
        if (rank == i % 2) {
            // Moja kolej — wyślij
            ++ping_pong_count;
            MPI_Send(&ping_pong_count, 1, MPI_INT, partner, 0, MPI_COMM_WORLD);
            std::cout << rank << " wysłał: " << ping_pong_count << "\n";
        } else {
            // Odbierz
            MPI_Recv(&ping_pong_count, 1, MPI_INT, partner, 0,
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            std::cout << rank << " odebrał: " << ping_pong_count << "\n";
        }
    }

    MPI_Finalize();
    return 0;
}
```

### 4.3 `MPI_Status` — informacje o wiadomości

```cpp
MPI_Status status;
MPI_Recv(buf, MAX_SIZE, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG,
         MPI_COMM_WORLD, &status);

int source = status.MPI_SOURCE;   // kto wysłał?
int tag    = status.MPI_TAG;      // jaki tag?

int actual_count;
MPI_Get_count(&status, MPI_INT, &actual_count);  // ile elementów przyszło?
```

### 4.4 Typy danych MPI

| Typ C/C++ | Typ MPI |
|-----------|---------|
| `int` | `MPI_INT` |
| `long` | `MPI_LONG` |
| `float` | `MPI_FLOAT` |
| `double` | `MPI_DOUBLE` |
| `char` | `MPI_CHAR` |
| `unsigned` | `MPI_UNSIGNED` |
| `long long` | `MPI_LONG_LONG` |
| `bool` | `MPI_C_BOOL` |

### 4.5 Deadlock — klasyczna pułapka!

```cpp
// ✗ DEADLOCK: oba procesy czekają na odbiór, nikt nie wysyła!
if (rank == 0) {
    MPI_Recv(&data, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, &status);
    MPI_Send(&mydata, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
} else {
    MPI_Recv(&data, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
    MPI_Send(&mydata, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
}

// ✓ POPRAWNE: odwrotna kolejność Send/Recv
if (rank == 0) {
    MPI_Send(&mydata, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
    MPI_Recv(&data, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, &status);
} else {
    MPI_Recv(&data, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
    MPI_Send(&mydata, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
}

// ✓ ALTERNATYWA: MPI_Sendrecv — bezpieczna wymiana
MPI_Sendrecv(&mydata, 1, MPI_INT, partner, 0,
             &data, 1, MPI_INT, partner, 0,
             MPI_COMM_WORLD, &status);
```

---

## 5. Komunikacja nieblokująca

### 5.1 Po co komunikacja nieblokująca?

```
Blokująca (MPI_Send/Recv):
  Proces czeka → marnuje czas CPU → nie nakłada obliczeń z komunikacją

Nieblokująca (MPI_Isend/Irecv):
  Proces inicjuje operację → robi coś pożytecznego → sprawdza wynik
  
  NAKŁADANIE obliczeń i komunikacji = lepsza wydajność!
```

### 5.2 `MPI_Isend` / `MPI_Irecv`

```cpp
MPI_Request request;

// Inicjuj wysyłanie — NIE CZEKAJ na zakończenie
MPI_Isend(&data, count, MPI_DOUBLE, dest, tag, MPI_COMM_WORLD, &request);

// ... rób coś pożytecznego w międzyczasie ...

// Teraz czekaj na zakończenie
MPI_Wait(&request, &status);
// Po Wait: data można bezpiecznie modyfikować
```

### 5.3 Pełny przykład — nakładanie obliczeń

```cpp
#include <mpi.h>
#include <vector>

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    const int N = 1000000;
    std::vector<double> data(N, rank * 1.0);
    std::vector<double> recv_buf(N);

    int partner = (rank + 1) % size;
    int source  = (rank - 1 + size) % size;

    MPI_Request send_req, recv_req;

    // Rozpocznij komunikację (nieblokujące)
    MPI_Isend(data.data(), N, MPI_DOUBLE, partner, 0,
              MPI_COMM_WORLD, &send_req);
    MPI_Irecv(recv_buf.data(), N, MPI_DOUBLE, source, 0,
              MPI_COMM_WORLD, &recv_req);

    // W MIĘDZYCZASIE: obliczenia lokalne (nie dotykaj data ani recv_buf!)
    double local_sum = 0.0;
    for (int i = 0; i < N; ++i) {
        local_sum += data[i] * data[i];  // ← NIE modyfikuj data!
    }

    // Czekaj na zakończenie komunikacji
    MPI_Wait(&send_req, MPI_STATUS_IGNORE);
    MPI_Wait(&recv_req, MPI_STATUS_IGNORE);

    // Teraz można bezpiecznie używać recv_buf
    double remote_sum = 0.0;
    for (int i = 0; i < N; ++i) {
        remote_sum += recv_buf[i];
    }

    MPI_Finalize();
    return 0;
}
```

### 5.4 `MPI_Test` — sprawdzenie bez czekania

```cpp
MPI_Request request;
MPI_Irecv(buf, count, MPI_INT, source, tag, comm, &request);

int flag = 0;
while (!flag) {
    MPI_Test(&request, &flag, &status);  // flag=1 gdy gotowe
    // ... inne obliczenia ...
}
// Dane gotowe w buf
```

### 5.5 `MPI_Waitall` — czekanie na wiele operacji

```cpp
std::vector<MPI_Request> requests(2 * (size - 1));
int req_idx = 0;

// Wyślij do wszystkich sąsiadów
for (int i = 0; i < size; ++i) {
    if (i != rank) {
        MPI_Isend(&mydata, 1, MPI_INT, i, 0, MPI_COMM_WORLD, &requests[req_idx++]);
        MPI_Irecv(&recv[i], 1, MPI_INT, i, 0, MPI_COMM_WORLD, &requests[req_idx++]);
    }
}

// Czekaj na WSZYSTKIE
MPI_Waitall(req_idx, requests.data(), MPI_STATUSES_IGNORE);
```

---

## 6. Operacje kolektywne (collective)

### 6.1 Przegląd operacji kolektywnych

```
MPI_Bcast        → Jeden do wszystkich (broadcast)
MPI_Scatter      → Rozdziel dane między procesy
MPI_Gather       → Zbierz dane od procesów do jednego
MPI_Allgather    → Zbierz dane — każdy dostaje pełny wynik
MPI_Alltoall     → Każdy do każdego (transpozycja)
MPI_Reduce       → Redukcja do jednego procesu
MPI_Allreduce    → Redukcja — każdy dostaje wynik
MPI_Barrier      → Synchronizacja (czekaj na wszystkich)
```

### 6.2 `MPI_Bcast` — rozgłoszenie

```cpp
int data;
if (rank == 0) {
    data = 42;  // root ma dane
}

// Wszyscy wywołują Bcast — root wysyła, reszta odbiera
MPI_Bcast(&data, 1, MPI_INT, 0, MPI_COMM_WORLD);
//         ↑buf   ↑count      ↑root

// Teraz KAŻDY proces ma data == 42
```

```
Przed Bcast:          Po Bcast:
  P0: [42]              P0: [42]
  P1: [??]              P1: [42]
  P2: [??]              P2: [42]
  P3: [??]              P3: [42]
```

### 6.3 `MPI_Scatter` — podział danych

```cpp
std::vector<int> sendbuf;
if (rank == 0) {
    sendbuf = {10, 20, 30, 40};  // 4 elementy, 4 procesy
}

int recvbuf;  // każdy dostanie 1 element
MPI_Scatter(sendbuf.data(), 1, MPI_INT,   // co wysyłam (root)
            &recvbuf, 1, MPI_INT,          // co odbieram (każdy)
            0, MPI_COMM_WORLD);            // root = 0

// P0: recvbuf=10, P1: recvbuf=20, P2: recvbuf=30, P3: recvbuf=40
```

```
Scatter (root=P0):
  P0: [10, 20, 30, 40]  →  P0: [10]
                             P1: [20]
                             P2: [30]
                             P3: [40]
```

### 6.4 `MPI_Gather` — zbieranie danych

```cpp
int myvalue = rank * 10;  // P0:0, P1:10, P2:20, P3:30

std::vector<int> gathered;
if (rank == 0) {
    gathered.resize(size);
}

MPI_Gather(&myvalue, 1, MPI_INT,        // co wysyłam (każdy)
           gathered.data(), 1, MPI_INT,  // gdzie zbieram (root)
           0, MPI_COMM_WORLD);           // root = 0

// Na P0: gathered = {0, 10, 20, 30}
```

```
Gather (root=P0):
  P0: [0]                    P0: [0, 10, 20, 30]
  P1: [10]              →    
  P2: [20]                   
  P3: [30]                   
```

### 6.5 `MPI_Allgather` — zbieranie do wszystkich

```cpp
int myvalue = rank;
std::vector<int> all_values(size);

MPI_Allgather(&myvalue, 1, MPI_INT,
              all_values.data(), 1, MPI_INT,
              MPI_COMM_WORLD);

// KAŻDY proces ma all_values = {0, 1, 2, 3}
```

### 6.6 `MPI_Alltoall` — transpozycja

```cpp
// Każdy proces wysyła inne dane do każdego
std::vector<int> sendbuf(size);  // sendbuf[i] → pójdzie do procesu i
std::vector<int> recvbuf(size);  // recvbuf[i] → przyszło od procesu i

for (int i = 0; i < size; ++i) {
    sendbuf[i] = rank * 100 + i;
}

MPI_Alltoall(sendbuf.data(), 1, MPI_INT,
             recvbuf.data(), 1, MPI_INT,
             MPI_COMM_WORLD);
```

```
Alltoall (4 procesy):
  Wysyłam:              Odbieram:
  P0: [00,01,02,03]    P0: [00,100,200,300]
  P1: [100,101,102,103] P1: [01,101,201,301]
  P2: [200,201,202,203] P2: [02,102,202,302]
  P3: [300,301,302,303] P3: [03,103,203,303]
```

### 6.7 `MPI_Barrier` — synchronizacja globalna

```cpp
// Wszyscy czekają aż KAŻDY dotrze do tego punktu
MPI_Barrier(MPI_COMM_WORLD);

// Użyteczne np. do pomiaru czasu
MPI_Barrier(MPI_COMM_WORLD);
double start = MPI_Wtime();
// ... obliczenia ...
MPI_Barrier(MPI_COMM_WORLD);
double end = MPI_Wtime();
if (rank == 0) {
    std::cout << "Czas: " << end - start << " s\n";
}
```

---

## 7. Redukcja i operacje globalne

### 7.1 `MPI_Reduce`

```cpp
double local_sum = compute_partial_sum(rank);
double global_sum;

MPI_Reduce(&local_sum, &global_sum, 1, MPI_DOUBLE,
           MPI_SUM, 0, MPI_COMM_WORLD);
//         ↑sendbuf  ↑recvbuf  ↑count ↑type  ↑op  ↑root

// Tylko P0 ma poprawny wynik w global_sum!
```

### 7.2 `MPI_Allreduce` — wynik u wszystkich

```cpp
double local_sum = compute_partial_sum(rank);
double global_sum;

MPI_Allreduce(&local_sum, &global_sum, 1, MPI_DOUBLE,
              MPI_SUM, MPI_COMM_WORLD);

// KAŻDY proces ma poprawny global_sum
```

### 7.3 Dostępne operacje redukcji

| Operacja MPI | Opis |
|-------------|------|
| `MPI_SUM` | Suma |
| `MPI_PROD` | Iloczyn |
| `MPI_MAX` | Maksimum |
| `MPI_MIN` | Minimum |
| `MPI_MAXLOC` | Max + pozycja |
| `MPI_MINLOC` | Min + pozycja |
| `MPI_LAND` | Logiczne AND |
| `MPI_LOR` | Logiczne OR |
| `MPI_LXOR` | Logiczne XOR |
| `MPI_BAND` | Bitowe AND |
| `MPI_BOR` | Bitowe OR |
| `MPI_BXOR` | Bitowe XOR |

### 7.4 Przykład — obliczanie π metodą Monte Carlo

```cpp
#include <mpi.h>
#include <random>
#include <iostream>

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    const long long total_points = 100000000LL;
    const long long points_per_proc = total_points / size;

    // Każdy proces ma inny seed
    std::mt19937_64 rng(42 + rank);
    std::uniform_real_distribution<double> dist(0.0, 1.0);

    long long local_inside = 0;
    for (long long i = 0; i < points_per_proc; ++i) {
        double x = dist(rng);
        double y = dist(rng);
        if (x * x + y * y <= 1.0) {
            ++local_inside;
        }
    }

    long long global_inside;
    MPI_Reduce(&local_inside, &global_inside, 1, MPI_LONG_LONG,
               MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        double pi = 4.0 * global_inside / total_points;
        std::cout << "π ≈ " << pi << "\n";
    }

    MPI_Finalize();
    return 0;
}
```

### 7.5 `MPI_Scan` — prefiks (scan / prefix sum)

```cpp
int local_val = rank + 1;  // P0:1, P1:2, P2:3, P3:4
int prefix_sum;

MPI_Scan(&local_val, &prefix_sum, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);

// P0: prefix_sum=1
// P1: prefix_sum=3  (1+2)
// P2: prefix_sum=6  (1+2+3)
// P3: prefix_sum=10 (1+2+3+4)
```

---

## 8. Typy danych MPI i typy pochodne

### 8.1 Po co typy pochodne?

```
Problem: Masz strukturę z różnymi polami, albo chcesz wysłać
         nieciągłe elementy tablicy (np. kolumnę macierzy).
         
MPI NIE wie jak Twoja struktura wygląda w pamięci!
→ Musisz opisać układ pamięci za pomocą TYPU POCHODNEGO
```

### 8.2 `MPI_Type_contiguous` — ciągły blok

```cpp
// Wysłanie 3 kolejnych double jako jeden "element"
MPI_Datatype vec3_type;
MPI_Type_contiguous(3, MPI_DOUBLE, &vec3_type);
MPI_Type_commit(&vec3_type);

double vec[3] = {1.0, 2.0, 3.0};
MPI_Send(vec, 1, vec3_type, dest, 0, MPI_COMM_WORLD);

MPI_Type_free(&vec3_type);  // zwolnij po użyciu
```

### 8.3 `MPI_Type_vector` — bloki z przeskokami (stride)

```cpp
// Kolumna macierzy NxM (elementy co M):
// matrix[0][col], matrix[1][col], ..., matrix[N-1][col]
int N = 100, M = 50;

MPI_Datatype column_type;
MPI_Type_vector(N,         // ile bloków
                1,         // elementów w bloku
                M,         // stride (odległość między blokami)
                MPI_DOUBLE,
                &column_type);
MPI_Type_commit(&column_type);

// Wyślij kolumnę 5:
MPI_Send(&matrix[0][5], 1, column_type, dest, 0, MPI_COMM_WORLD);

MPI_Type_free(&column_type);
```

### 8.4 `MPI_Type_create_struct` — struktury

```cpp
struct Particle {
    double x, y, z;    // pozycja
    double vx, vy, vz; // prędkość
    int id;            // identyfikator
};

MPI_Datatype particle_type;

// Ile pól:
int blocklengths[] = {3, 3, 1};
MPI_Datatype types[] = {MPI_DOUBLE, MPI_DOUBLE, MPI_INT};

// Offsety pól w strukturze:
MPI_Aint offsets[3];
offsets[0] = offsetof(Particle, x);
offsets[1] = offsetof(Particle, vx);
offsets[2] = offsetof(Particle, id);

MPI_Type_create_struct(3, blocklengths, offsets, types, &particle_type);
MPI_Type_commit(&particle_type);

// Teraz można wysyłać struktury jak zwykłe dane:
Particle p = {1.0, 2.0, 3.0, 0.1, 0.2, 0.3, 42};
MPI_Send(&p, 1, particle_type, dest, 0, MPI_COMM_WORLD);

MPI_Type_free(&particle_type);
```

### 8.5 `MPI_Pack` / `MPI_Unpack` — alternatywa

```cpp
// Pakowanie różnych danych do bufora
char buffer[1000];
int position = 0;

int n = 5;
double values[5] = {1.1, 2.2, 3.3, 4.4, 5.5};

MPI_Pack(&n, 1, MPI_INT, buffer, 1000, &position, MPI_COMM_WORLD);
MPI_Pack(values, n, MPI_DOUBLE, buffer, 1000, &position, MPI_COMM_WORLD);

MPI_Send(buffer, position, MPI_PACKED, dest, 0, MPI_COMM_WORLD);

// Odbiorca:
MPI_Recv(buffer, 1000, MPI_PACKED, source, 0, MPI_COMM_WORLD, &status);
position = 0;
MPI_Unpack(buffer, 1000, &position, &n, 1, MPI_INT, MPI_COMM_WORLD);
MPI_Unpack(buffer, 1000, &position, values, n, MPI_DOUBLE, MPI_COMM_WORLD);
```

---

## 9. Komunikatory i grupy procesów

### 9.1 Czym jest komunikator?

```
Komunikator = kontekst komunikacji + grupa procesów

MPI_COMM_WORLD — domyślny, zawiera WSZYSTKIE procesy

Po co własne komunikatory?
  - Podział pracy na zespoły (np. wiersze vs kolumny macierzy)
  - Izolacja komunikacji (biblioteki nie kolidują z głównym programem)
  - Topologie (siatki, grafy)
```

### 9.2 `MPI_Comm_split` — podział na podgrupy

```cpp
// Podziel 8 procesów na 2 grupy po 4:
int color = rank / 4;      // P0-P3 → kolor 0, P4-P7 → kolor 1
int key   = rank % 4;      // rank wewnątrz nowej grupy

MPI_Comm row_comm;
MPI_Comm_split(MPI_COMM_WORLD, color, key, &row_comm);

int row_rank, row_size;
MPI_Comm_rank(row_comm, &row_rank);
MPI_Comm_size(row_comm, &row_size);

// Broadcast WEWNĄTRZ grupy — nie wpływa na inne grupy!
MPI_Bcast(&data, 1, MPI_INT, 0, row_comm);

MPI_Comm_free(&row_comm);  // zwolnij po użyciu
```

### 9.3 Przykład — siatka 2D

```cpp
int dims = 2;  // 2D grid
int rows = size / 4, cols = 4;

int row_color = rank / cols;
int col_color = rank % cols;

MPI_Comm row_comm, col_comm;
MPI_Comm_split(MPI_COMM_WORLD, row_color, rank, &row_comm);  // wiersz
MPI_Comm_split(MPI_COMM_WORLD, col_color, rank, &col_comm);  // kolumna

// Reduce po wierszach (niezależnie w każdym wierszu)
MPI_Reduce(&local, &row_sum, 1, MPI_DOUBLE, MPI_SUM, 0, row_comm);

// Reduce po kolumnach (niezależnie w każdej kolumnie)
MPI_Reduce(&local, &col_sum, 1, MPI_DOUBLE, MPI_SUM, 0, col_comm);

MPI_Comm_free(&row_comm);
MPI_Comm_free(&col_comm);
```

---

## 10. Topologie wirtualne

### 10.1 Topologia kartezjańska (siatka)

```cpp
// Stwórz siatkę 2D (np. 4x4 = 16 procesów)
int dims[2] = {4, 4};
int periods[2] = {0, 0};  // 0 = bez zawijania, 1 = toroidalna
int reorder = 1;           // pozwól MPI zmienić ranki dla wydajności

MPI_Comm cart_comm;
MPI_Cart_create(MPI_COMM_WORLD, 2, dims, periods, reorder, &cart_comm);

// Moja pozycja w siatce:
int coords[2];
MPI_Cart_coords(cart_comm, rank, 2, coords);
// np. rank=5 → coords=[1,1] w siatce 4x4

// Rank sąsiada w kierunku (dimension=0, direction=+1):
int left, right, up, down;
MPI_Cart_shift(cart_comm, 0, 1, &up, &down);     // wymiar 0 (wiersz)
MPI_Cart_shift(cart_comm, 1, 1, &left, &right);  // wymiar 1 (kolumna)
// Jeśli sąsiad nie istnieje (brzeg) → MPI_PROC_NULL

// Komunikacja z sąsiadami:
MPI_Sendrecv(&mydata, 1, MPI_DOUBLE, right, 0,
             &from_left, 1, MPI_DOUBLE, left, 0,
             cart_comm, MPI_STATUS_IGNORE);
```

### 10.2 Zastosowanie — stencil 2D (wymiana granic)

```cpp
// Każdy proces ma lokalny kawałek macierzy + "ghost cells" (brzegi)
// Wymiana ghost cells z 4 sąsiadami:

MPI_Sendrecv(&local[1][1],   1, row_type, up,    0,
             &local[N+1][1], 1, row_type, down,  0,
             cart_comm, MPI_STATUS_IGNORE);

MPI_Sendrecv(&local[N][1],   1, row_type, down,  1,
             &local[0][1],   1, row_type, up,    1,
             cart_comm, MPI_STATUS_IGNORE);

MPI_Sendrecv(&local[1][1],   1, col_type, left,  2,
             &local[1][M+1], 1, col_type, right, 2,
             cart_comm, MPI_STATUS_IGNORE);

MPI_Sendrecv(&local[1][M],   1, col_type, right, 3,
             &local[1][0],   1, col_type, left,  3,
             cart_comm, MPI_STATUS_IGNORE);
```

---

## 11. Komunikacja jednostronna (RMA / One-sided)

### 11.1 Idea RMA

```
Klasycznie: Send + Recv — OBA procesy muszą uczestniczyć
RMA:        Put/Get     — JEDEN proces czyta/pisze w pamięci INNEGO

  Proces 0:  MPI_Put(dane, ..., okno procesu 1)
  Proces 1:  (nic nie robi — nie musi wiedzieć!)
```

### 11.2 Okna pamięci (`MPI_Win`)

```cpp
double* shared_data;
MPI_Win win;

// Każdy proces tworzy okno (expose fragment pamięci):
MPI_Win_allocate(N * sizeof(double), sizeof(double),
                 MPI_INFO_NULL, MPI_COMM_WORLD,
                 &shared_data, &win);

// Inicjalizuj lokalne dane
for (int i = 0; i < N; ++i) {
    shared_data[i] = rank * 1000 + i;
}
```

### 11.3 `MPI_Put` / `MPI_Get`

```cpp
// Fence synchronization (prostsza)
MPI_Win_fence(0, win);

if (rank == 0) {
    double buf[10];
    // Odczytaj 10 elementów z okna procesu 1, offset=0
    MPI_Get(buf, 10, MPI_DOUBLE, 1, 0, 10, MPI_DOUBLE, win);
}

MPI_Win_fence(0, win);
// Po fence: dane są gotowe
```

### 11.4 Lock/Unlock — passive target

```cpp
if (rank == 0) {
    double value = 3.14;

    MPI_Win_lock(MPI_LOCK_EXCLUSIVE, 1, 0, win);  // zablokuj okno P1
    MPI_Put(&value, 1, MPI_DOUBLE, 1, 5, 1, MPI_DOUBLE, win);  // pisz pod offset 5
    MPI_Win_unlock(1, win);  // odblokuj
}
// Proces 1 nie musi nic robić — Put dzieje się "za jego plecami"
```

### 11.5 `MPI_Accumulate` — atomowe operacje RMA

```cpp
MPI_Win_lock(MPI_LOCK_SHARED, target, 0, win);
MPI_Accumulate(&local_val, 1, MPI_DOUBLE, target, 0, 1, MPI_DOUBLE,
               MPI_SUM, win);
MPI_Win_unlock(target, win);
// Atomowo: okno[target][0] += local_val
```

```cpp
// Sprzątanie
MPI_Win_free(&win);
```

---

## 12. Równoległe wejście/wyjście (MPI I/O)

### 12.1 Po co MPI I/O?

```
Problem: 1000 procesów chce zapisać wyniki do jednego pliku.
  
  Rozwiązanie naiwne: Zbierz wszystko na P0 → P0 pisze do pliku
    ✗ Wąskie gardło pamięci i I/O na jednym procesie
  
  Rozwiązanie MPI I/O: Każdy proces pisze SWOJĄ CZĘŚĆ bezpośrednio
    ✓ Równoległy dostęp do pliku, skalowalne
```

### 12.2 Podstawowy zapis

```cpp
MPI_File fh;
MPI_File_open(MPI_COMM_WORLD, "output.bin",
              MPI_MODE_CREATE | MPI_MODE_WRONLY,
              MPI_INFO_NULL, &fh);

// Każdy proces pisze w swoim miejscu w pliku
MPI_Offset offset = rank * N * sizeof(double);
MPI_File_write_at(fh, offset, local_data, N, MPI_DOUBLE, MPI_STATUS_IGNORE);

MPI_File_close(&fh);
```

### 12.3 Widok pliku (file view) — elegancka wersja

```cpp
MPI_File fh;
MPI_File_open(MPI_COMM_WORLD, "output.bin",
              MPI_MODE_CREATE | MPI_MODE_WRONLY,
              MPI_INFO_NULL, &fh);

// Ustaw widok: każdy proces widzi plik jako ciągłe dane
MPI_Offset disp = rank * N * sizeof(double);
MPI_File_set_view(fh, disp, MPI_DOUBLE, MPI_DOUBLE, "native", MPI_INFO_NULL);

// Teraz pisz jakbyś pisał od początku
MPI_File_write(fh, local_data, N, MPI_DOUBLE, MPI_STATUS_IGNORE);

MPI_File_close(&fh);
```

---

## 13. Pułapki i dobre praktyki

### 13.1 Najczęstsze błędy

```
✗ DEADLOCK: Oba procesy wywołują Recv przed Send
  → Rozwiązanie: Sendrecv, lub nieblokujące Isend/Irecv

✗ NIEZGODNE TYPY: Wysyłam MPI_INT, odbieram MPI_DOUBLE
  → Undefined behavior! MPI NIE sprawdza typów w runtime

✗ NIEZGODNE ROZMIARY: Wysyłam 100 elementów, bufor na 50
  → Buffer overflow, MPI_ERR_TRUNCATE

✗ BRAK Finalize: Zasoby nie zwolnione, mogą wyciekać procesy

✗ OPERACJE KOLEKTYWNE nie wywołane przez WSZYSTKIE procesy:
  → Wszystkie procesy w komunikatorze MUSZĄ wywołać np. Bcast!
  → Inaczej: zawiśnięcie (hang)

✗ TAG MISMATCH: Send z tag=0, Recv z tag=1
  → Recv nigdy nie dopasuje wiadomości → deadlock

✗ Używanie danych PRZED Wait (po Isend/Irecv):
  → Race condition — dane mogą być jeszcze w trakcie transferu!
```

### 13.2 Dobre praktyki

```
✓ Używaj MPI_Sendrecv zamiast ręcznych par Send/Recv
  → Unika deadlocków automatycznie

✓ Używaj operacji kolektywnych zamiast pętli Send/Recv
  → Bcast, Reduce, Scatter są zoptymalizowane (drzewo, ring, ...)

✓ Preferuj komunikację nieblokującą
  → Nakładaj obliczenia z komunikacją

✓ Mierz czas za pomocą MPI_Wtime() (nie clock())
  → MPI_Wtime() jest przenośne i ma dobrą rozdzielczość

✓ Używaj MPI_PROC_NULL jako "null device"
  → Send/Recv do MPI_PROC_NULL to no-op (upraszcza logikę brzegów)

✓ Testuj z różną liczbą procesów
  → Wiele bugów pojawia się dopiero przy -np 1, -np 3, -np 7

✓ default(none) w OpenMP; w MPI: sprawdzaj kody błędów
  → int err = MPI_Send(...); if (err != MPI_SUCCESS) { ... }
```

### 13.3 Pomiar wydajności

```cpp
MPI_Barrier(MPI_COMM_WORLD);  // synchronizuj start
double t0 = MPI_Wtime();

// ... obliczenia + komunikacja ...

MPI_Barrier(MPI_COMM_WORLD);  // synchronizuj koniec
double t1 = MPI_Wtime();

double local_time = t1 - t0;
double max_time;
MPI_Reduce(&local_time, &max_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

if (rank == 0) {
    std::cout << "Czas najwolniejszego procesu: " << max_time << " s\n";
}
```

---

## 14. Wzorce projektowe i zadania

### 14.1 Wzorzec Master-Worker

```cpp
const int TAG_WORK = 1;
const int TAG_RESULT = 2;
const int TAG_DONE = 3;

if (rank == 0) {
    // MASTER: rozdzielaj zadania dynamicznie
    int next_task = 0;
    int active_workers = size - 1;

    // Wyślij początkowe zadania
    for (int w = 1; w < size && next_task < total_tasks; ++w) {
        MPI_Send(&next_task, 1, MPI_INT, w, TAG_WORK, MPI_COMM_WORLD);
        ++next_task;
    }

    // Odbieraj wyniki, wysyłaj nowe zadania
    while (active_workers > 0) {
        MPI_Status status;
        double result;
        MPI_Recv(&result, 1, MPI_DOUBLE, MPI_ANY_SOURCE, TAG_RESULT,
                 MPI_COMM_WORLD, &status);
        int worker = status.MPI_SOURCE;

        // Zapisz wynik...
        process_result(result);

        if (next_task < total_tasks) {
            MPI_Send(&next_task, 1, MPI_INT, worker, TAG_WORK, MPI_COMM_WORLD);
            ++next_task;
        } else {
            MPI_Send(nullptr, 0, MPI_INT, worker, TAG_DONE, MPI_COMM_WORLD);
            --active_workers;
        }
    }
} else {
    // WORKER: wykonuj zadania aż dostaniesz TAG_DONE
    while (true) {
        int task_id;
        MPI_Status status;
        MPI_Recv(&task_id, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        if (status.MPI_TAG == TAG_DONE) break;

        double result = do_work(task_id);
        MPI_Send(&result, 1, MPI_DOUBLE, 0, TAG_RESULT, MPI_COMM_WORLD);
    }
}
```

### 14.2 Wzorzec Domain Decomposition (podział domeny)

```cpp
// Każdy proces odpowiada za fragment tablicy
int local_N = N / size;
int start = rank * local_N;
int end   = (rank == size - 1) ? N : start + local_N;

std::vector<double> local(local_N + 2);  // +2 ghost cells

// Iteracje (np. Jacobi):
for (int iter = 0; iter < max_iter; ++iter) {
    // Wymiana granic z sąsiadami
    int left  = (rank > 0) ? rank - 1 : MPI_PROC_NULL;
    int right = (rank < size - 1) ? rank + 1 : MPI_PROC_NULL;

    MPI_Sendrecv(&local[1], 1, MPI_DOUBLE, left, 0,
                 &local[local_N + 1], 1, MPI_DOUBLE, right, 0,
                 MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    MPI_Sendrecv(&local[local_N], 1, MPI_DOUBLE, right, 1,
                 &local[0], 1, MPI_DOUBLE, left, 1,
                 MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    // Obliczenia lokalne (stencil)
    for (int i = 1; i <= local_N; ++i) {
        new_local[i] = 0.5 * (local[i-1] + local[i+1]);
    }
    std::swap(local, new_local);
}
```

### 14.3 📝 Zadania do samodzielnego rozwiązania

#### Zadanie 1: Ring — przekazywanie wiadomości w pierścieniu

```
Napisz program, w którym N procesów przekazuje wiadomość w pierścieniu:
  P0 → P1 → P2 → ... → P(N-1) → P0

Każdy proces dodaje do wiadomości swój rank.
Na końcu P0 powinien wyświetlić sumę wszystkich ranków.
Użyj MPI_Sendrecv aby uniknąć deadlocka.
```

#### Zadanie 2: Rozproszone sortowanie (odd-even transposition sort)

```
Zaimplementuj równoległy algorytm sortowania:
1. Każdy proces ma tablicę N/P elementów (posortowaną lokalnie)
2. W parzystych fazach: procesy parzyste wymieniają z sąsiadem +1
3. W nieparzystych fazach: procesy nieparzyste wymieniają z sąsiadem +1
4. Wymiana = merge dwóch posortowanych ciągów, zachowaj dolną/górną połowę
5. Po P fazach — tablica jest posortowana globalnie

Wskazówki:
  - Użyj MPI_Sendrecv do wymiany danych
  - std::merge + podział na połowy
```

#### Zadanie 3: Mnożenie macierzy — algorytm Cannon'a

```
Zaimplementuj mnożenie macierzy C = A * B na siatce P×P procesów:
1. Stwórz topologię kartezjańską 2D
2. Wstępne przesunięcie (initial alignment)
3. P kroków: lokalne mnożenie + shift A w lewo + shift B w górę
4. Zbierz wynik

Wymagania:
  - MPI_Cart_create, MPI_Cart_shift
  - MPI_Sendrecv do przesunięć
  - Sprawdź wynik porównując z sekwencyjnym mnożeniem
```

#### Zadanie 4: Równanie ciepła 2D

```
Rozwiąż numerycznie równanie ciepła na siatce 2D (metoda jawna):
  u[i][j]_new = u[i][j] + α * Δt/Δx² * (u[i+1][j] + u[i-1][j] + 
                                           u[i][j+1] + u[i][j-1] - 4*u[i][j])

1. Podziel siatkę na poziome pasy (domain decomposition)
2. Każdy proces ma swój pas + ghost rows
3. W każdym kroku: wymień ghost rows → oblicz nowy krok
4. Co K kroków: zbierz pełny wynik na P0 (MPI_Gather)

Wskazówki:
  - Warunki brzegowe: Dirichlet (stała temperatura na brzegach)
  - Warunek początkowy: punkt gorący w środku
```

#### Zadanie 5: Obliczanie całki (Gauss-MPI)

```
Oblicz numerycznie całkę:
  ∫₀¹ 4/(1+x²) dx = π

Metoda: prostokątów (lub Simpsona) z N = 10⁸ punktów.
1. Każdy proces liczy swoją część przedziału [a_i, b_i]
2. MPI_Reduce(MPI_SUM) zbiera wynik
3. Zmierz przyspieszenie (speedup) dla 1, 2, 4, 8, 16 procesów
4. Oblicz efektywność E = S(p) / p
```

#### Zadanie 6: Game of Life z MPI

```
Zaimplementuj Game of Life na planszy NxN z podziałem na P procesów:
1. Każdy proces zarządza pasem wierszy (N/P wierszy)
2. Wymiana ghost rows co krok
3. Warunek brzegowy: toroidalny (periods={1,1})
4. Co T kroków: zapis stanu do pliku za pomocą MPI I/O

Bonus: użyj komunikacji nieblokującej do nakładania obliczeń
       wewnętrznych komórek z wymianą granic
```

#### Zadanie 7: MapReduce — zliczanie słów

```
Zaimplementuj prosty MapReduce:
1. P0 (master) wczytuje duży plik tekstowy
2. MPI_Scatter — rozdziela fragmenty tekstu
3. Każdy process zlicza wystąpienia słów (std::map<string,int>)
4. MPI_Gather — zbierz wyniki na P0
5. P0 łączy (merge) mapy i wypisuje Top-10 słów

Wskazówki:
  - Serializacja map: MPI_Pack / MPI_Unpack
  - Alternatywa: wyślij jako pary (string, count)
```

---

## 15. Ściąga — najważniejsze funkcje

```cpp
// ── INICJALIZACJA / FINALIZACJA ─────────────────────────────────────
MPI_Init(&argc, &argv);
MPI_Finalize();
MPI_Comm_rank(comm, &rank);
MPI_Comm_size(comm, &size);
MPI_Wtime();                        // zegarek (double, sekundy)

// ── KOMUNIKACJA PUNKT-PUNKT (blokująca) ─────────────────────────────
MPI_Send(buf, count, type, dest, tag, comm);
MPI_Recv(buf, count, type, source, tag, comm, &status);
MPI_Sendrecv(sendbuf, scount, stype, dest, stag,
             recvbuf, rcount, rtype, source, rtag, comm, &status);

// ── KOMUNIKACJA PUNKT-PUNKT (nieblokująca) ──────────────────────────
MPI_Isend(buf, count, type, dest, tag, comm, &request);
MPI_Irecv(buf, count, type, source, tag, comm, &request);
MPI_Wait(&request, &status);
MPI_Waitall(count, requests[], statuses[]);
MPI_Test(&request, &flag, &status);

// ── OPERACJE KOLEKTYWNE ─────────────────────────────────────────────
MPI_Bcast(buf, count, type, root, comm);
MPI_Scatter(sendbuf, scount, stype, recvbuf, rcount, rtype, root, comm);
MPI_Gather(sendbuf, scount, stype, recvbuf, rcount, rtype, root, comm);
MPI_Allgather(sendbuf, scount, stype, recvbuf, rcount, rtype, comm);
MPI_Alltoall(sendbuf, scount, stype, recvbuf, rcount, rtype, comm);
MPI_Reduce(sendbuf, recvbuf, count, type, op, root, comm);
MPI_Allreduce(sendbuf, recvbuf, count, type, op, comm);
MPI_Scan(sendbuf, recvbuf, count, type, op, comm);
MPI_Barrier(comm);

// ── TYPY POCHODNE ───────────────────────────────────────────────────
MPI_Type_contiguous(count, oldtype, &newtype);
MPI_Type_vector(count, blocklength, stride, oldtype, &newtype);
MPI_Type_create_struct(count, blocklengths[], offsets[], types[], &newtype);
MPI_Type_commit(&type);
MPI_Type_free(&type);

// ── KOMUNIKATORY ────────────────────────────────────────────────────
MPI_Comm_split(comm, color, key, &newcomm);
MPI_Comm_free(&comm);

// ── TOPOLOGIE ───────────────────────────────────────────────────────
MPI_Cart_create(comm, ndims, dims[], periods[], reorder, &cart_comm);
MPI_Cart_coords(comm, rank, ndims, coords[]);
MPI_Cart_shift(comm, direction, disp, &src, &dest);

// ── RMA (ONE-SIDED) ─────────────────────────────────────────────────
MPI_Win_allocate(size, disp_unit, info, comm, &baseptr, &win);
MPI_Win_fence(assert, win);
MPI_Put(buf, count, type, target_rank, offset, count, type, win);
MPI_Get(buf, count, type, target_rank, offset, count, type, win);
MPI_Win_lock(lock_type, rank, assert, win);
MPI_Win_unlock(rank, win);
MPI_Win_free(&win);

// ── I/O ─────────────────────────────────────────────────────────────
MPI_File_open(comm, filename, amode, info, &fh);
MPI_File_write_at(fh, offset, buf, count, type, &status);
MPI_File_read_at(fh, offset, buf, count, type, &status);
MPI_File_set_view(fh, disp, etype, filetype, datarep, info);
MPI_File_close(&fh);

// ── STAŁE SPECJALNE ─────────────────────────────────────────────────
MPI_COMM_WORLD          // domyślny komunikator (wszystkie procesy)
MPI_ANY_SOURCE          // odbierz od kogokolwiek
MPI_ANY_TAG             // odbierz z dowolnym tagiem
MPI_PROC_NULL           // "czarna dziura" — Send/Recv to no-op
MPI_STATUS_IGNORE       // nie potrzebuję informacji o statusie
MPI_STATUSES_IGNORE     // j.w. dla Waitall
MPI_IN_PLACE            // bufor in-place (np. w Allreduce)
```

---

## Podsumowanie

```
MPI to standard do programowania równoległego z pamięcią ROZPROSZONĄ.

PODSTAWY:
  MPI_Init / MPI_Finalize       → wejście/wyjście ze świata MPI
  MPI_Send / MPI_Recv           → wyślij/odbierz wiadomość
  MPI_Comm_rank / MPI_Comm_size → kim jestem? ilu nas jest?

KOLEKTYWNE:
  Bcast       → jeden do wszystkich
  Scatter     → podziel dane
  Gather      → zbierz dane
  Reduce      → agregacja (sum, max, min, ...)
  Allreduce   → agregacja — wynik u wszystkich

ZAAWANSOWANE:
  Isend/Irecv + Wait   → komunikacja nieblokująca (overlap)
  Typy pochodne        → struktury, kolumny macierzy
  Comm_split           → podgrupy procesów
  Cart_create          → topologie (siatki, torusy)
  RMA (Put/Get)        → komunikacja jednostronna
  MPI I/O              → równoległy zapis/odczyt plików

PUŁAPKI:
  ✗ Deadlock: oba procesy czekają na Recv → używaj Sendrecv
  ✗ Type mismatch: wysyłam INT, odbieram DOUBLE → undefined behavior
  ✗ Brak Finalize → wycieki zasobów
  ✗ Niezbalansowane kolektywy → hang
  ✗ Dane przed Wait po Irecv → race condition
```
