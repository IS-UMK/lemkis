# Sanitizery — TSan i ASan
## Narzędzia do wykrywania błędów współbieżności i pamięci w C++

---

## 1. Czym są sanitizery?

**Sanitizery** to narzędzia diagnostyczne **wbudowane w kompilator** (GCC, Clang),
które instrumentują Twój kod — dodają automatyczne sprawdzenia w runtime'ie.
Nie wymagają żadnych zmian w kodzie źródłowym — wystarczy **jedna flaga kompilacji**.

```bash
# Zwykła kompilacja — błędy NIEWIDOCZNE:
g++ -g program.cpp -o program
./program          # "działa" (ale ma UB — tykająca bomba)

# Kompilacja z sanitizerem — błędy WIDOCZNE:
g++ -fsanitize=thread -g program.cpp -o program
./program          # RAPORT: data race w linii 42!
```

### Dostępne sanitizery

| Sanitizer | Flaga | Wykrywa | Spowolnienie |
|-----------|-------|---------|:---:|
| **TSan** (Thread Sanitizer) | `-fsanitize=thread` | Data race'y | ~5–15× |
| **ASan** (Address Sanitizer) | `-fsanitize=address` | Use-after-free, buffer overflow, memory leak | ~2–3× |
| **UBSan** (Undefined Behavior) | `-fsanitize=undefined` | Signed overflow, null deref, shift errors | ~1.2× |
| **MSan** (Memory Sanitizer) | `-fsanitize=memory` | Odczyt niezainicjalizowanej pamięci | ~3× |

> **⚠️ TSan i ASan NIE MOGĄ działać jednocześnie** — uruchamiaj osobno.
> ASan + UBSan **mogą** działać razem.

---

## 2. TSan — Thread Sanitizer

Wykrywa **data race'y** — jednoczesny dostęp do pamięci z co najmniej jednym zapisem,
bez synchronizacji.

### 2.1 Kompilacja i uruchomienie

```bash
# GCC
g++ -fsanitize=thread -g -O1 program.cpp -o program -pthread
./program

# Clang
clang++ -fsanitize=thread -g -O1 program.cpp -o program -pthread
./program
```

> **Dlaczego `-O1`?** Bez optymalizacji raporty mogą być mniej dokładne
> (zbyt dużo szumu). `-O1` to dobry kompromis między dokładnością a czytelnością.
> **`-g`** dodaje informacje debugowe — dzięki temu raport pokaże numery linii.

### 2.2 Przykład: data race na liczniku

```cpp
// plik: data_race.cpp
#include <iostream>
#include <thread>

int counter = 0;  // ❌ zwykły int, brak synchronizacji

int main() {
    std::thread t1([] {
        for (int i = 0; i < 100'000; ++i)
            counter++;  // ❌ data race!
    });

    std::thread t2([] {
        for (int i = 0; i < 100'000; ++i)
            counter++;  // ❌ data race!
    });

    t1.join();
    t2.join();

    std::cout << "Counter = " << counter << "\n";
    // Oczekiwane: 200'000
    // Rzeczywiste: losowa wartość < 200'000 (ale UB = cokolwiek)
}
```

```bash
g++ -fsanitize=thread -g -O1 data_race.cpp -o data_race -pthread
./data_race
```

**Raport TSan:**

```
==================
WARNING: ThreadSanitizer: data race (pid=12345)
  Write of size 4 at 0x000000601080 by thread T2:
    #0 main::$_1::operator()() at data_race.cpp:12:13

  Previous write of size 4 at 0x000000601080 by thread T1:
    #0 main::$_0::operator()() at data_race.cpp:7:13

  Location is global 'counter' of size 4 at 0x000000601080
    (data_race+0x601080)

  Thread T1 (tid=12346, running) created by main thread at:
    #0 pthread_create
    #1 std::thread::_M_start_thread
    #2 main at data_race.cpp:6:21

  Thread T2 (tid=12347, running) created by main thread at:
    #0 pthread_create
    #1 std::thread::_M_start_thread
    #2 main at data_race.cpp:11:21

SUMMARY: ThreadSanitizer: data race data_race.cpp:12:13 in
         main::$_1::operator()()
==================
Counter = 143726
ThreadSanitizer: reported 1 warnings
```

### Jak czytać raport TSan?

```
WARNING: ThreadSanitizer: data race
                          ^^^^^^^^^ TYP BŁĘDU

  Write of size 4 at 0x...601080 by thread T2:
  ^^^^^              ^^^^^^^^^^^    ^^^^^^^^^
  operacja           adres pamięci  który wątek
    #0 main::$_1::operator()() at data_race.cpp:12:13
                                   ^^^^^^^^^^^^^^^^^
                                   DOKŁADNA LINIA KODU

  Previous write of size 4 at 0x...601080 by thread T1:
  ^^^^^^^^^^^^^^
  KOLIDUJĄCY dostęp (inny wątek, ta sama lokalizacja)
    #0 main::$_0::operator()() at data_race.cpp:7:13

  Location is global 'counter'
                      ^^^^^^^
                      NAZWA ZMIENNEJ z data race'em
```

### 2.3 Przykład: data race na kontenerze

```cpp
// plik: vector_race.cpp
#include <iostream>
#include <thread>
#include <vector>

int main() {
    std::vector<int> vec;

    std::thread writer([&vec] {
        for (int i = 0; i < 1000; ++i)
            vec.push_back(i);       // ❌ modyfikacja bez locka
    });

    std::thread reader([&vec] {
        for (int i = 0; i < 1000; ++i) {
            if (!vec.empty())
                volatile int x = vec.back();  // ❌ odczyt bez locka
        }
    });

    writer.join();
    reader.join();
}
```

```bash
g++ -fsanitize=thread -g -O1 vector_race.cpp -o vector_race -pthread
./vector_race
```

```
WARNING: ThreadSanitizer: data race (pid=12345)
  Read of size 8 at 0x7ffc... by thread T2:
    #0 std::vector<int>::back() at /usr/include/c++/12/bits/stl_vector.h:1140
    #1 main::$_1::operator()() at vector_race.cpp:14:37

  Previous write of size 8 at 0x7ffc... by thread T1:
    #0 std::vector<int>::push_back() at /usr/include/c++/12/bits/stl_vector.h:1198
    #1 main::$_0::operator()() at vector_race.cpp:9:17
```

> TSan pokazuje data race **wewnątrz** `std::vector` — w implementacji
> `push_back()` i `back()`. Widać że problem jest w **Twoim kodzie**
> (linie 9 i 14), nie w bibliotece standardowej.

### 2.4 Przykład: data race w Thread Pool (ćwiczenie 2)

```cpp
// plik: pool_race.cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <vector>
#include <functional>

class SimplePool {
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;
    std::mutex mutex_;
    std::condition_variable cv_;
    bool shutdown_ = false;

public:
    explicit SimplePool(size_t n) {
        for (size_t i = 0; i < n; ++i)
            workers_.emplace_back([this] {
                while (true) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(mutex_);
                        cv_.wait(lock, [this] {
                            return !tasks_.empty() || shutdown_;
                        });
                        if (shutdown_ && tasks_.empty()) return;
                        task = std::move(tasks_.front());
                        tasks_.pop();
                    }
                    task();
                }
            });
    }

    ~SimplePool() {
        { std::lock_guard<std::mutex> lock(mutex_); shutdown_ = true; }
        cv_.notify_all();
        for (auto& w : workers_) w.join();
    }

    void submit(std::function<void()> f) {
        { std::lock_guard<std::mutex> lock(mutex_); tasks_.push(std::move(f)); }
        cv_.notify_one();
    }
};

int main() {
    SimplePool pool(4);
    int counter = 0;  // ❌ zwykły int!

    for (int i = 0; i < 10'000; ++i) {
        pool.submit([&counter] {
            counter++;  // ❌ data race!
        });
    }
}
```

```bash
g++ -fsanitize=thread -g -O1 pool_race.cpp -o pool_race -pthread
./pool_race
```

```
WARNING: ThreadSanitizer: data race (pid=54321)
  Write of size 4 at 0x7ffc...0a4c by thread T3:
    #0 main::$_0::operator()() at pool_race.cpp:49:13

  Previous write of size 4 at 0x7ffc...0a4c by thread T1:
    #0 main::$_0::operator()() at pool_race.cpp:49:13

  Location is stack of main thread.
```

→ TSan namierza linię `counter++` (49) i mówi że T1 i T3 piszą jednocześnie.

---

## 3. ASan — Address Sanitizer

Wykrywa **błędy pamięci** — use-after-free, buffer overflow, stack overflow,
memory leak, use-after-return.

### 3.1 Kompilacja i uruchomienie

```bash
# GCC
g++ -fsanitize=address -g -O1 program.cpp -o program

# Clang
clang++ -fsanitize=address -g -O1 program.cpp -o program

# ASan + UBSan razem (MOŻNA łączyć):
g++ -fsanitize=address,undefined -g -O1 program.cpp -o program

# Dodatkowe opcje:
# Wykrywanie use-after-return (domyślnie wyłączone w GCC):
ASAN_OPTIONS=detect_stack_use_after_return=1 ./program

# Wykrywanie memory leaks (domyślnie włączone w Clang):
ASAN_OPTIONS=detect_leaks=1 ./program
```

### 3.2 Przykład: use-after-free (dangling reference z ćwiczenia 3)

```cpp
// plik: dangling.cpp
#include <iostream>
#include <functional>
#include <vector>
#include <numeric>

std::function<void()> make_task() {
    std::vector<int> dane = {1, 2, 3, 4, 5};

    return [&dane] {  // ❌ referencja do zmiennej lokalnej!
        int suma = std::accumulate(dane.begin(), dane.end(), 0);
        std::cout << "Suma = " << suma << "\n";
    };
    // ← dane ZNISZCZONE po return
}

int main() {
    auto task = make_task();
    task();  // ❌ dane już nie istnieje!
}
```

```bash
g++ -fsanitize=address -g -O1 dangling.cpp -o dangling
ASAN_OPTIONS=detect_stack_use_after_return=1 ./dangling
```

```
=================================================================
==54321==ERROR: AddressSanitizer: stack-use-after-return on address
0x7f1234500020 at pc 0x... bp 0x... sp 0x...
READ of size 8 at 0x7f1234500020 thread T0
    #0 0x... in make_task()::$_0::operator()() dangling.cpp:10
    #1 0x... in std::function<void()>::operator()()
    #2 0x... in main dangling.cpp:16

Address 0x7f1234500020 is located in stack of thread T0 at offset
32 in frame
    #0 0x... in make_task() dangling.cpp:6

  This frame has 1 object(s):
    [32, 56) 'dane' (line 7) <== Memory access at offset 32 is
    inside this variable
HINT: this may be a false positive if your program uses some custom
stack unwind mechanism
SUMMARY: AddressSanitizer: stack-use-after-return dangling.cpp:10
=================================================================
```

### Jak czytać raport ASan?

```
ERROR: AddressSanitizer: stack-use-after-return
                         ^^^^^^^^^^^^^^^^^^^^^^ TYP BŁĘDU

READ of size 8 at 0x7f... thread T0
^^^^                       ^^^^^^^^^
operacja                   który wątek
    #0 ... in make_task()::$_0::operator()() dangling.cpp:10
                                              ^^^^^^^^^^^^^
                                              LINIA BŁĘDU

  This frame has 1 object(s):
    [32, 56) 'dane' (line 7) <== Memory access at offset 32 is
              ^^^^  ^^^^^^^^     inside this variable
              NAZWA ZMIENNEJ    LINIA DEKLARACJI
```

### 3.3 Przykład: buffer overflow

```cpp
// plik: overflow.cpp
#include <iostream>

int main() {
    int tablica[5] = {10, 20, 30, 40, 50};

    // ❌ Odczyt poza tablicą!
    for (int i = 0; i <= 5; ++i) {    // <= zamiast <
        std::cout << tablica[i] << "\n";
    }
}
```

```bash
g++ -fsanitize=address -g -O1 overflow.cpp -o overflow
./overflow
```

```
10
20
30
40
50
=================================================================
==12345==ERROR: AddressSanitizer: stack-buffer-overflow on address
0x7ffc...001c at pc 0x... bp 0x... sp 0x...
READ of size 4 at 0x7ffc...001c thread T0
    #0 0x... in main overflow.cpp:7

Address 0x7ffc...001c is located in stack of thread T0 at offset
44 in frame
    #0 0x... in main overflow.cpp:3

  This frame has 1 object(s):
    [16, 36) 'tablica' (line 4)
    ^^^^^^^^^^^^^^^^^^
    tablica ma 20 bajtów (5 × 4), ale odczyt na offset 40 → POZA!
=================================================================
```

### 3.4 Przykład: memory leak

```cpp
// plik: leak.cpp
#include <iostream>

void funkcja() {
    int* p = new int[1000];  // alokacja
    p[0] = 42;
    std::cout << p[0] << "\n";
    // ❌ brak delete[] p!
}

int main() {
    for (int i = 0; i < 100; ++i)
        funkcja();
}
```

```bash
g++ -fsanitize=address -g -O1 leak.cpp -o leak
./leak
```

```
42
42
...

=================================================================
==12345==ERROR: LeakSanitizer: detected memory leaks

Direct leak of 400000 byte(s) in 100 object(s) allocated from:
    #0 0x... in operator new[](unsigned long)
    #1 0x... in funkcja() leak.cpp:4
    #2 0x... in main leak.cpp:11

SUMMARY: AddressSanitizer: 400000 byte(s) leaked in 100 allocation(s).
=================================================================
```

→ ASan mówi: **400'000 bajtów** wyciekło, alokowanych w `leak.cpp:4`,
wywoływanych z `main` w linii 11.

### 3.5 Przykład: dangling `this` (ćwiczenie 10)

```cpp
// plik: dangling_this.cpp
#include <iostream>
#include <thread>
#include <chrono>
#include <functional>

class Sensor {
    int id_;
    double reading_ = 0.0;

public:
    Sensor(int id) : id_(id) {}

    std::function<void()> make_read_task() {
        return [this] {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            reading_ = 42.0;  // ❌ this może być dangling!
            std::cout << "Sensor " << id_ << ": " << reading_ << "\n";
        };
    }
};

int main() {
    std::function<void()> task;

    {
        Sensor s(1);
        task = s.make_read_task();
    }   // ← s ZNISZCZONE

    std::thread t(task);
    t.join();
}
```

```bash
g++ -fsanitize=address -g -O1 dangling_this.cpp -o dangling_this -pthread
./dangling_this
```

```
=================================================================
==12345==ERROR: AddressSanitizer: stack-use-after-scope on address
0x7ffc...0040 at pc 0x... bp 0x... sp 0x...
WRITE of size 8 at 0x7ffc...0040 thread T1
    #0 0x... in Sensor::make_read_task()::$_0::operator()()
       dangling_this.cpp:16

Address 0x7ffc...0040 is located in stack of thread T0 at offset
64 in frame
    #0 0x... in main dangling_this.cpp:22

  This frame has 1 object(s):
    [48, 64) 's' (line 24) <== Memory access at offset 64 is
    inside this variable
=================================================================
```

→ ASan mówi: wątek T1 pisze do zmiennej `s` (linia 24), która już wyszła ze scope.

---

## 4. Porównanie i zasady użycia

### Co wykrywa który sanitizer?

| Błąd | TSan | ASan | Bez sanitizera |
|------|:---:|:---:|:---:|
| Data race (dwa wątki piszą do `int`) | ✅ | ❌ | ❌ „działa" |
| Use-after-free / dangling reference | ❌ | ✅ | ❌ „działa" |
| Buffer overflow (tablica) | ❌ | ✅ | ❌ „działa" |
| Memory leak (`new` bez `delete`) | ❌ | ✅ | ❌ „działa" |
| Dangling `this` | ❌ | ✅ | ❌ „działa" |
| Signed integer overflow | ❌ | ❌ (UBSan!) | ❌ „działa" |

### Ograniczenia

| | TSan | ASan |
|---|---|---|
| **Spowolnienie** | 5–15× | 2–3× |
| **Zużycie RAM** | 5–10× | 2–3× |
| **Można łączyć razem?** | ❌ **NIE** z ASan | ❌ **NIE** z TSan |
| **Można z UBSan?** | ✅ TAK | ✅ TAK |
| **Kompilatory** | GCC, Clang | GCC, Clang, MSVC (częściowo) |
| **Windows** | ⚠️ Słabe wsparcie | ✅ MSVC: `/fsanitize=address` |
| **Fałszywe alarmy?** | Bardzo rzadko | Bardzo rzadko |

### ⚠️ Kluczowe ograniczenie

Sanitizery wykrywają błędy tylko **gdy kod się faktycznie wykona**.
Jeśli data race występuje raz na 1000 uruchomień — TSan go wykryje
**tylko jeśli akurat w tym uruchomieniu** oba wątki trafią na siebie.

> **Wniosek:** Uruchamiaj testy z sanitizerami **wielokrotnie** i z różnymi
> danymi wejściowymi. Im więcej ścieżek kodu pokryjesz, tym więcej błędów znajdziesz.

---

## 5. Przepis na codzienną pracę

### 5.1 Makefile z sanitizerami

```makefile
CXX      = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -g

# Targety:
all: program

program: main.cpp
	$(CXX) $(CXXFLAGS) -O2 main.cpp -o program -pthread

# Debug z Thread Sanitizer:
tsan: main.cpp
	$(CXX) $(CXXFLAGS) -O1 -fsanitize=thread main.cpp -o program_tsan -pthread
	./program_tsan

# Debug z Address Sanitizer:
asan: main.cpp
	$(CXX) $(CXXFLAGS) -O1 -fsanitize=address,undefined main.cpp -o program_asan -pthread
	ASAN_OPTIONS=detect_stack_use_after_return=1 ./program_asan

# Uruchom OBA (osobno):
sanitize: tsan asan
	@echo "✅ Oba sanitizery przeszły bez błędów!"

clean:
	rm -f program program_tsan program_asan
```

```bash
# Codzienne użycie:
make tsan        # szukaj data race'ów
make asan        # szukaj błędów pamięci
make sanitize    # oba naraz (osobne uruchomienia)
```

### 5.2 CMake z sanitizerami

```cmake
# CMakeLists.txt

cmake_minimum_required(VERSION 3.14)
project(MojProjekt CXX)
set(CMAKE_CXX_STANDARD 20)

add_executable(program main.cpp)
target_link_libraries(program pthread)

# Opcja: sanitizer
option(SANITIZER "Enable sanitizer (tsan, asan, ubsan)" "")

if(SANITIZER STREQUAL "tsan")
    message(STATUS "Thread Sanitizer ENABLED")
    target_compile_options(program PRIVATE -fsanitize=thread -g -O1)
    target_link_options(program PRIVATE -fsanitize=thread)

elseif(SANITIZER STREQUAL "asan")
    message(STATUS "Address Sanitizer ENABLED")
    target_compile_options(program PRIVATE -fsanitize=address,undefined -g -O1)
    target_link_options(program PRIVATE -fsanitize=address,undefined)
endif()
```

```bash
# Użycie:
cmake -B build-tsan -DSANITIZER=tsan && cmake --build build-tsan && ./build-tsan/program
cmake -B build-asan -DSANITIZER=asan && cmake --build build-asan && ./build-asan/program
```

### 5.3 Skrypt CI (GitHub Actions)

```yaml
# .github/workflows/sanitizers.yml
name: Sanitizers

on: [push, pull_request]

jobs:
  tsan:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      - name: Build with TSan
        run: g++ -std=c++20 -fsanitize=thread -g -O1 main.cpp -o program -pthread
      - name: Run with TSan
        run: ./program

  asan:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      - name: Build with ASan
        run: g++ -std=c++20 -fsanitize=address,undefined -g -O1 main.cpp -o program -pthread
      - name: Run with ASan
        env:
          ASAN_OPTIONS: detect_stack_use_after_return=1:detect_leaks=1
        run: ./program
```

---

## 6. Ćwiczenia

### Ćwiczenie 1: Znajdź data race TSanem 🔍

Skompiluj i uruchom poniższy kod **z TSan**. Ile data race'ów zgłosi?
Napraw je i uruchom ponownie — TSan powinien być cichy.

```cpp
// plik: cwiczenie_tsan.cpp
#include <iostream>
#include <thread>
#include <vector>

int balance = 1000;
bool done = false;

void deposit() {
    for (int i = 0; i < 1000; ++i)
        balance += 10;
    done = true;
}

void withdraw() {
    while (!done) {}  // czekaj
    for (int i = 0; i < 1000; ++i)
        balance -= 5;
}

int main() {
    std::thread t1(deposit);
    std::thread t2(withdraw);
    t1.join();
    t2.join();
    std::cout << "Balance = " << balance << "\n";
}
```

```bash
g++ -fsanitize=thread -g -O1 cwiczenie_tsan.cpp -o cwiczenie_tsan -pthread
./cwiczenie_tsan
```

<details>
<summary>Ile data race'ów i jakie?</summary>

TSan zgłosi **2 data race'y**:

1. **`balance`** — oba wątki piszą/czytają bez synchronizacji
2. **`done`** — `deposit` pisze `true`, `withdraw` czyta w pętli — bez synchronizacji

**Naprawa:**
```cpp
std::atomic<int> balance{1000};
std::atomic<bool> done{false};
```

</details>

---

### Ćwiczenie 2: Znajdź błąd pamięci ASanem 🔍

Skompiluj i uruchom **z ASan**. Jaki błąd zgłosi?

```cpp
// plik: cwiczenie_asan.cpp
#include <iostream>
#include <thread>
#include <vector>
#include <numeric>

void process(std::vector<int>& out) {
    std::vector<int> local = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    std::thread worker([&local, &out] {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        int sum = std::accumulate(local.begin(), local.end(), 0);
        out.push_back(sum);
    });

    worker.detach();  // ← uwaga na to!
}

int main() {
    std::vector<int> results;
    process(results);

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    for (int x : results)
        std::cout << x << "\n";
}
```

```bash
g++ -fsanitize=address -g -O1 cwiczenie_asan.cpp -o cwiczenie_asan -pthread
ASAN_OPTIONS=detect_stack_use_after_return=1 ./cwiczenie_asan
```

<details>
<summary>Jaki błąd i dlaczego?</summary>

ASan zgłosi **stack-use-after-return** na zmiennej `local`.

`worker.detach()` oznacza: wątek działa dalej nawet po wyjściu z `process()`.
Ale `local` jest na stosie `process()` — ginie po return.
Wątek worker czyta `local` po 50ms — ale `local` już nie istnieje.

Dodatkowy problem: `results` jest referencją do zmiennej w `main()` — to akurat
przeżyje (bo `main` czeka 200ms), ale `local` nie.

**Naprawa:**
```cpp
// Opcja A: join zamiast detach
worker.join();

// Opcja B: przechwytuj local przez kopię
std::thread worker([local, &out] { ... });
//                  ^^^^^ bez &
```

</details>

---

### Ćwiczenie 3: Porównaj — z i bez sanitizera 📊

Weź dowolne ćwiczenie z poprzednich zajęć (np. ćwiczenie 2 z Thread Pool —
data race na liczniku) i skompiluj **trzy razy**:

```bash
# 1. Bez sanitizera:
g++ -g -O2 program.cpp -o prog_normal -pthread

# 2. Z TSan:
g++ -g -O1 -fsanitize=thread program.cpp -o prog_tsan -pthread

# 3. Z ASan:
g++ -g -O1 -fsanitize=address program.cpp -o prog_asan -pthread
```

Zmierz czas wykonania każdej wersji i wypełnij tabelę:

| Wersja | Czas | Błędy znalezione? | Wynik programu |
|:---:|:---:|:---:|:---:|
| Bez sanitizera | ___ ms | | counter = ___ |
| Z TSan | ___ ms | | counter = ___ |
| Z ASan | ___ ms | | counter = ___ |

**Pytania:**
- O ile wolniejsza jest wersja z TSan vs bez?
- Czy wersja bez sanitizera **kiedykolwiek** zgłosiła błąd?
- Który sanitizer znalazł problem?

---

## 7. Podsumowanie

### Reguła kciuka

```
Kompilujesz kod w C++?
│
├─ Czy to build produkcyjny?
│  └─ NIE → kompiluj z sanitizerami!
│     ├─ Kod wielowątkowy? → -fsanitize=thread
│     ├─ Każdy kod C++?    → -fsanitize=address,undefined
│     └─ CI/testy?         → uruchamiaj OBA (osobne buildy)
│
└─ TAK (produkcja)?
   └─ Bez sanitizerów (overhead za duży)
      Ale: uruchamiaj testy z sanitizerami PRZED deployem!
```

### Najważniejsze komendy

```bash
# Data race'y:
g++ -fsanitize=thread -g -O1 plik.cpp -o prog -pthread && ./prog

# Błędy pamięci + UB:
g++ -fsanitize=address,undefined -g -O1 plik.cpp -o prog -pthread && ./prog

# Nie łącz TSan z ASan! Osobne uruchomienia:
make tsan && make asan
```

---

## Literatura

- Google Sanitizers documentation: [github.com/google/sanitizers](https://github.com/google/sanitizers)
- Clang docs: [AddressSanitizer](https://clang.llvm.org/docs/AddressSanitizer.html),
  [ThreadSanitizer](https://clang.llvm.org/docs/ThreadSanitizer.html)
- CppCon 2015: Kostya Serebryany, *Building and using sanitizers*
