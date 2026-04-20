# Pula Wątków — Ćwiczenia diagnostyczne
## Znajdź problem w kodzie 🔍

---

## Ćwiczenie 0: Bez puli wątków

Poniższy serwer tworzy nowy wątek dla każdego przychodzącego żądania.
Uruchom mentalnie (lub na komputerze) dla dużej liczby żądań.

```cpp
#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <atomic>

std::atomic<int> active_threads{0};

void handle_request(int request_id) {
    active_threads++;
    std::cout << "Request " << request_id
              << " | aktywne wątki: " << active_threads << "\n";

    // Symulacja przetwarzania (I/O, baza danych, obliczenia...)
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    active_threads--;
}

int main() {
    std::vector<std::thread> threads;

    // Symulacja: 100'000 żądań naraz
    for (int i = 0; i < 100'000; ++i) {
        threads.emplace_back(handle_request, i);
    }

    for (auto& t : threads) t.join();
    std::cout << "Wszystkie żądania obsłużone.\n";
}
```

**Pytania:**

1. Co się stanie gdy uruchomisz ten program?
2. Ile pamięci (w przybliżeniu) potrzeba na 100'000 wątków?
3. Jaki będzie wpływ na wydajność systemu operacyjnego?
4. Jak rozwiązałbyś ten problem?

<details>
<summary>Odpowiedź</summary>

**Co pójdzie nie tak:**

| Problem | Wyjaśnienie |
|---------|-------------|
| **Wyczerpanie pamięci** | Każdy wątek potrzebuje ~1–8 MB stosu. 100'000 × 8 MB = **800 GB!** System odmówi tworzenia nowych wątków (`std::system_error`) |
| **Thrashing** | Nawet jeśli OS stworzy kilka tysięcy wątków, scheduler musi je ciągle przełączać. Context switch ≈ 1–10 μs, ale przy tysiącach wątków → CPU spędza więcej czasu na przełączaniu niż na pracy |
| **Koszt tworzenia** | Tworzenie wątku to syscall (Linux: `clone`, Windows: `CreateThread`) ≈ 5–50 μs per wątek. Dla 100k wątków to ~1–5 sekund samego tworzenia |
| **Cache pollution** | Każdy nowy wątek ma zimny cache → ciągłe cache missy |

**Rozwiązanie:** **Pula wątków** — tworzymy np. 8 wątków (= liczba rdzeni) i kolejkujemy zadania. Żadne dodatkowe wątki nie powstają.

```
Bez puli:   100'000 wątków → 800 GB RAM → crash
Z pulą (8): 8 wątków → 64 MB RAM → OK, zadania czekają w kolejce
```

</details>

---

## Ćwiczenie 1: Zadanie w zadaniu

```cpp
ThreadPool pool(2);

auto f1 = pool.submit([&pool] {
    auto f2 = pool.submit([] { return 10; });
    return f2.get() + 5;
});

auto f3 = pool.submit([&pool] {
    auto f4 = pool.submit([] { return 20; });
    return f4.get() + 5;
});

std::cout << f1.get() + f3.get() << "\n";
```

**Pytanie:** Czy ten program wypisze `40`? Czy może się zachować inaczej?
Jeśli tak — przy jakich warunkach? Czy zmiana rozmiaru puli pomoże?

<details>
<summary>Odpowiedź</summary>

**Deadlock (thread pool exhaustion).**

```
Worker 1: wykonuje f1 → submit(f2) → f2.get() → 💤 CZEKA
Worker 2: wykonuje f3 → submit(f4) → f4.get() → 💤 CZEKA
Kolejka:  [f2] [f4]  ← NIKT ich nie wykona — oba workery śpią!

CYKL:
  Worker 1 czeka na f2 (w kolejce)
  Worker 2 czeka na f4 (w kolejce)
  f2 i f4 czekają na wolnego workera → BRAK → DEADLOCK ☠️
```

Pool ma **2 wątki**, oba zajęte przez f1 i f3, które czekają na f2 i f4 — ale f2 i f4 siedzą w kolejce bez szans na wykonanie.

**Naprawa:**

- Zwiększenie puli do ≥ 4 wątków pomoże **w tym przypadku**, ale nie rozwiązuje problemu ogólnie — przy większej głębokości zagnieżdżenia deadlock wróci
- Właściwe rozwiązanie: **nie rób `future.get()` wewnątrz zadania** — zamiast tego submituj kontynuację, albo użyj osobnych pooli dla outer/inner tasków

</details>

---

## Ćwiczenie 2: Współdzielony licznik

```cpp
ThreadPool pool(4);
int counter = 0;

for (int i = 0; i < 100'000; ++i) {
    pool.submit([&counter] {
        counter++;
    });
}

// ... po zakończeniu wszystkich tasków ...
std::cout << "Counter = " << counter << "\n";
```

**Pytanie:** Czy `counter` będzie równy `100'000`? Uruchom mentalnie
na 4 wątkach — co może się wydarzyć między odczytem a zapisem `counter`?

<details>
<summary>Odpowiedź</summary>

**Data race na `counter` — Undefined Behavior!**

`counter++` to trzy operacje: (1) odczytaj counter, (2) dodaj 1, (3) zapisz counter.
Cztery wątki robią to jednocześnie bez synchronizacji:

```
Worker 1: odczyt counter=5 → +1 → zapis counter=6
Worker 2: odczyt counter=5 → +1 → zapis counter=6  ← TEN SAM odczyt!
                                                       stracony inkrement!
```

Wynik będzie **mniejszy** niż 100'000 (typowo ~25'000–90'000), ale formalnie
to **Undefined Behavior** — program może zrobić dosłownie cokolwiek.

**Naprawa:**

```cpp
std::atomic<int> counter{0};
// ...
pool.submit([&counter] {
    counter.fetch_add(1);
});
```

Albo: każdy task zwraca wynik przez `future`, sumujemy w jednym wątku.

</details>

---

## Ćwiczenie 3: Lambda i referencja

```cpp
void schedule_work(ThreadPool& pool) {
    std::vector<int> dane = {1, 2, 3, 4, 5};

    pool.submit([&dane] {
        int suma = 0;
        for (int x : dane) suma += x;
        std::cout << "Suma = " << suma << "\n";
    });
}

int main() {
    ThreadPool pool(4);
    schedule_work(pool);

    std::this_thread::sleep_for(std::chrono::seconds(1));
}
```

**Pytanie:** Czy program wypisze `Suma = 15`? Zwróć uwagę na czas
życia zmiennej `dane` i moment wykonania zadania.

<details>
<summary>Odpowiedź</summary>

**Dangling reference — Use-After-Free!**

```
schedule_work():
  dane = [1, 2, 3, 4, 5]        ← na stosie
  submit(lambda z &dane)         ← lambda ma REFERENCJĘ do dane
  return                         ← dane ZNISZCZONE (stos zwinięty)

...chwilę później...

Worker: task()
  for (int x : dane)            ← dane nie istnieje! Śmieci w pamięci!
  💥 UB: crash, losowe wartości, format dysku...
```

Lambda przechwytuje `dane` przez **referencję** (`&dane`), ale `dane` jest zmienną lokalną
`schedule_work()` — ginie gdy funkcja się kończy. Task może wystartować **po** zniszczeniu `dane`.

**Naprawa:** Przechwytuj przez **kopię**:

```cpp
pool.submit([dane] {    // BEZ &  → kopia wektora
    int suma = 0;
    for (int x : dane) suma += x;
    std::cout << "Suma = " << suma << "\n";
});
```

Dla dużych obiektów — `std::shared_ptr`:
```cpp
auto dane = std::make_shared<std::vector<int>>(
    std::initializer_list<int>{1,2,3,4,5});
pool.submit([dane] {   // shared_ptr kopiowany — dane żyją dopóki task żyje
    // ...
});
```

</details>

---

## Ćwiczenie 4: Lambda i zmienna pętli

```cpp
ThreadPool pool(4);

for (int i = 0; i < 10; ++i) {
    pool.submit([&i] {
        std::cout << "Zadanie " << i << "\n";
    });
}
```

**Pytanie:** Jakie wartości `i` zostaną wypisane? Czy zobaczysz
`0, 1, 2, ..., 9`? A może coś innego?

<details>
<summary>Odpowiedź</summary>

**Dangling reference + data race na zmiennej pętli!**

Lambda przechwytuje `i` przez **referencję**. Wszystkie 10 lambd wskazuje na **tę samą zmienną `i`**,
która zmienia się w pętli i jest niszczona po wyjściu z pętli.

Typowy output (niedeterministyczny):
```
Zadanie 7
Zadanie 10    ← 10? Pętla idzie do 9!
Zadanie 10
Zadanie 10
...
```

Problemy:
1. **Data race:** pętla zmienia `i` (producent), taski czytają `i` (konsumenci) — bez synchronizacji
2. **Dangling reference:** po wyjściu z pętli `i` nie istnieje — UB
3. **Wyścig z wartością:** nawet bez UB, większość tasków zobaczy `i=10` (wartość po pętli)

**Naprawa:**

```cpp
for (int i = 0; i < 10; ++i) {
    pool.submit([i] {        // BEZ & → KOPIA wartości i
        std::cout << "Zadanie " << i << "\n";
    });
}
```

</details>

---

## Ćwiczenie 5: Wyjątek w zadaniu

```cpp
// Wersja ThreadPool z submit(std::function<void()>) — BEZ future:

class SimplePool {
    // ...
    void worker_loop() {
        while (true) {
            std::function<void()> task;
            {
                std::unique_lock<std::mutex> lock(mutex_);
                cv_.wait(lock, [this] { return !tasks_.empty() || shutdown_; });
                if (shutdown_ && tasks_.empty()) return;
                task = std::move(tasks_.front());
                tasks_.pop();
            }
            task();  // ← tutaj
        }
    }
};

// Użycie:
SimplePool pool(4);
pool.submit([] { throw std::runtime_error("Boom!"); });
pool.submit([] { std::cout << "Drugie zadanie\n"; });
```

**Pytanie:** Co się stanie z workerem który wykonuje pierwsze zadanie?
Czy drugie zadanie się wykona? Co się stanie z resztą programu?

<details>
<summary>Odpowiedź</summary>

**Wyjątek niezłapany w wątku → `std::terminate()` → cały program ginie!**

```
Worker 1: task() → throw "Boom!"
          → wyjątek opuszcza worker_loop()
          → wyjątek opuszcza funkcję wątku
          → std::terminate()
          → CAŁY PROGRAM KILLED ☠️
```

Standard C++ mówi: niezłapany wyjątek w wątku → `std::terminate()`.
**Nie** zabija tylko tego workera — **zabija cały proces**.

Drugie zadanie **nigdy się nie wykona**.

**Naprawa:** Łap wyjątki w `worker_loop()`:

```cpp
void worker_loop() {
    while (true) {
        // ... pobierz task ...
        try {
            task();
        } catch (const std::exception& e) {
            std::cerr << "Task failed: " << e.what() << "\n";
        } catch (...) {
            std::cerr << "Task failed: unknown exception\n";
        }
        // Worker ŻYJE — bierze następne zadanie ✅
    }
}
```

**Uwaga:** Jeśli używasz wersji `submit()` zwracającej `std::future`,
to `std::packaged_task` **sam łapie** wyjątek i przekazuje go przez `future.get()`.
Wtedy worker nie umiera. Ale w wersji bez `future` — musisz łapać ręcznie.

</details>

---

## Ćwiczenie 6: Kolejność destrukcji

```cpp
class DataProcessor {
    ThreadPool pool_;
    std::shared_ptr<std::vector<int>> data_;

public:
    DataProcessor()
        : pool_(4)
        , data_(std::make_shared<std::vector<int>>(1'000'000, 42))
    {}

    void process() {
        pool_.submit([this] {
            long long sum = 0;
            for (int x : *data_) sum += x;
            std::cout << "Suma = " << sum << "\n";
        });
    }
};

int main() {
    DataProcessor dp;
    dp.process();
}
```

**Pytanie:** Program kompiluje się i czasem działa poprawnie.
Ale czasem crashuje. Zwróć uwagę na **kolejność deklaracji**
memberów klasy — w jakiej kolejności C++ je niszczy?

<details>
<summary>Odpowiedź</summary>

**Use-after-free z powodu kolejności destrukcji!**

C++ niszczy membery klasy w **odwrotnej kolejności deklaracji**:

```
Deklaracja:
  (1) pool_       ← tworzony PIERWSZY
  (2) data_       ← tworzony DRUGI

Destrukcja (ODWROTNA):
  (1) ~data_      ← niszczony PIERWSZY → shared_ptr zwolniony!
  (2) ~pool_      ← niszczony DRUGI → join()... ale task JESZCZE DZIAŁA!
```

```
~DataProcessor():
  (1) ~data_ → shared_ptr reset → jeśli to ostatni → wektor ZWOLNIONY
  (2) ~pool_ → join() → czekamy na workera...
       └─ Worker: for (int x : *data_) → 💥 wektor nie istnieje!
```

**Naprawa:** Zmień kolejność deklaracji — pool **na końcu** (niszczony **pierwszy**):

```cpp
class DataProcessor {
    std::shared_ptr<std::vector<int>> data_;   // (1) niszczony DRUGI ← dane żyją
    ThreadPool pool_;                           // (2) niszczony PIERWSZY ← join() tutaj

    // ~pool_ → join() → taski kończą z żywym data_ ✅
    // ~data_ → bezpieczne zwolnienie ✅
};
```

Albo: lambda przechwytuje `shared_ptr` przez kopię (nie `this`):

```cpp
void process() {
    auto data_copy = data_;  // kopia shared_ptr → ref count +1
    pool_.submit([data_copy] {
        for (int x : *data_copy) // ...
    });
}
```

</details>

---

## Ćwiczenie 7: Wyścig z danymi po submit

```cpp
ThreadPool pool(4);
std::vector<int> data = {1, 2, 3, 4, 5};

auto future = pool.submit([&data] {
    int sum = 0;
    for (int x : data) sum += x;
    return sum;
});

data.clear();
data.push_back(999);

std::cout << "Wynik: " << future.get() << "\n";
```

**Pytanie:** Jaki będzie wynik? `15`? `999`? `0`? A może coś jeszcze innego?

<details>
<summary>Odpowiedź</summary>

**Data race — Undefined Behavior!**

`data` jest przechwycone przez **referencję**. Są dwa wątki operujące na `data` **bez synchronizacji**:

```
Możliwy scenariusz 1:
  Worker: sum += data[0] (=1)
  Main:   data.clear()        ← realokacja pamięci!
  Worker: sum += data[1]      ← 💥 iterator invalidated, UB!

Możliwy scenariusz 2:
  Main:   data.clear(); data.push_back(999);
  Worker: for (int x : data) → sum = 999
  Output: 999

Możliwy scenariusz 3:
  Worker: for (int x : data) → sum = 15
  Main:   data.clear();
  Output: 15

Formalnie: WSZYSTKIE scenariusze to UB, bo mamy data race.
```

`data.clear()` i `data.push_back()` mogą realokować pamięć wektora
**podczas gdy** worker iteruje po nim → crash, śmieci, cokolwiek.

**Naprawy:**

```cpp
// Opcja A: Przechwytuj przez kopię
auto future = pool.submit([data] { ... });  // kopia wektora

// Opcja B: Poczekaj na wynik PRZED modyfikacją
auto future = pool.submit([&data] { ... });
std::cout << future.get() << "\n";
data.clear();  // teraz bezpieczne — task skończony

// Opcja C: shared_ptr
auto data_ptr = std::make_shared<std::vector<int>>(
    std::initializer_list<int>{1,2,3,4,5});
auto future = pool.submit([data_ptr] { ... });
```

</details>

---

## Ćwiczenie 8: Niekontrolowana kolejka

```cpp
ThreadPool pool(2);

for (int i = 0; i < 100'000'000; ++i) {
    pool.submit([i] {
        // Ciężkie obliczenie — 100ms per task
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        if (i % 1'000'000 == 0)
            std::cout << "Zadanie " << i << " done\n";
    });
}

std::cout << "Wszystkie zadania zlecone!\n";
```

**Pytanie:** `submit()` jest szybkie (~1 μs), a wykonanie zadania trwa 100 ms.
Co się stanie z kolejką zadań w międzyczasie? Ile pamięci zajmie ta kolejka?
Ile czasu upłynie zanim pierwsze zadanie się wykona?

<details>
<summary>Odpowiedź</summary>

**Niekontrolowany wzrost kolejki → Out of Memory!**

```
Szybkość submit():     ~1 μs per task = 1'000'000 tasków/sekundę
Szybkość workerów:     100 ms per task × 2 workery = 20 tasków/sekundę
Dysproporcja:          50'000× ← producent jest 50 tysięcy razy szybszy!

Po 1 sekundzie:
  Zlecone:  1'000'000 tasków
  Wykonane: 20 tasków
  W kolejce: 999'980 tasków

sizeof(std::function<void()>) ≈ 32–64 bytes
100'000'000 × 64 B = ~6.4 GB RAM ← zanim cokolwiek się wykona!
```

Pętla `for` zdąży zlecić **wszystkie 100 milionów tasków** w kilka sekund,
bo `submit()` jest nieblokujące. Cała kolejka musi zmieścić się w RAM.

**Naprawa:** Ograniczona kolejka z **backpressure**:

```cpp
BoundedThreadPool pool(2, /*max_pending=*/ 1000);
// submit() BLOKUJE gdy >1000 tasków w kolejce
// Producent zwalnia do prędkości konsumentów ✅
```

</details>

---

## Ćwiczenie 9: Zagłodzenie krótkich zadań

```cpp
ThreadPool pool(2);

// Dwa bardzo długie zadania:
pool.submit([] {
    std::cout << "Heavy 1 START\n";
    std::this_thread::sleep_for(std::chrono::minutes(10));
    std::cout << "Heavy 1 DONE\n";
});

pool.submit([] {
    std::cout << "Heavy 2 START\n";
    std::this_thread::sleep_for(std::chrono::minutes(10));
    std::cout << "Heavy 2 DONE\n";
});

// Krytyczne zadanie — musi się wykonać SZYBKO:
auto urgent = pool.submit([] {
    return std::string("PILNA ODPOWIEDŹ");
});

std::cout << "Czekam na odpowiedź...\n";
std::cout << urgent.get() << "\n";  // ile tu poczekamy?
```

**Pytanie:** Ile czasu minie zanim zobaczymy `"PILNA ODPOWIEDŹ"`?
Czy istnieje sposób żeby pilne zadanie miało priorytet?

<details>
<summary>Odpowiedź</summary>

**Zagłodzenie (starvation) — pilne zadanie czeka 10 minut!**

```
Worker 1: [████████ heavy 10 min ██████████████████████████████]
Worker 2: [████████ heavy 10 min ██████████████████████████████]
Kolejka:  [urgent] 💤💤💤💤💤💤💤💤💤 (10 minut w kolejce!)
```

Oba wątki są zajęte przez 10-minutowe zadania. Pilne zadanie siedzi w kolejce
i **nie ma kto go wykonać**. `urgent.get()` zablokuje main thread na **10 minut**.

To nie jest deadlock — program w końcu się zakończy. Ale latencja jest katastrofalna.

**Naprawy:**

| Rozwiązanie | Opis |
|---|---|
| **Osobne poole** | `heavy_pool(2)` na długie taski, `fast_pool(2)` na pilne |
| **Priorytetowa kolejka** | `PriorityThreadPool` — pilne taski na górę kolejki |
| **Więcej wątków** | Ale to przesuwa problem, nie rozwiązuje |
| **Dziel długie taski** | Zamiast 1 taska × 10 min → 600 tasków × 1 s (z yield co sekundę) |

</details>

---

## Ćwiczenie 10: `this` w zniszczonym obiekcie

```cpp
class Sensor {
    ThreadPool& pool_;
    int sensor_id_;
    double last_reading_ = 0.0;

public:
    Sensor(ThreadPool& pool, int id) : pool_(pool), sensor_id_(id) {}

    void schedule_read() {
        pool_.submit([this] {
            // Symulacja odczytu sensora
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            last_reading_ = 42.0;
            std::cout << "Sensor " << sensor_id_ << ": " << last_reading_ << "\n";
        });
    }
};

int main() {
    ThreadPool pool(4);

    {
        Sensor s(pool, 1);
        s.schedule_read();
    }   // ← s ZNISZCZONE

    std::this_thread::sleep_for(std::chrono::seconds(2));
}
```

**Pytanie:** Lambda przechwytuje `this`. Co się stanie po wyjściu
z bloku `{}`? Task startuje po ~0 ms, ale odczyt trwa 500 ms.

<details>
<summary>Odpowiedź</summary>

**Dangling `this` — Use-After-Free!**

```
Blok {}:
  Sensor s na stosie
  schedule_read() → submit(lambda z this=&s)
  } ← s ZNISZCZONE (stos zwinięty)

~500ms później:
  Worker: this->last_reading_ = 42.0  ← this wskazuje na zwolnioną pamięć! 💥
  Worker: this->sensor_id_            ← śmieci
```

Lambda przechwytuje `this` — surowy wskaźnik na obiekt `Sensor`.
Obiekt jest na stosie i ginie po wyjściu z bloku. Worker operuje na
zwolnionej pamięci — Undefined Behavior.

**Naprawy:**

```cpp
// Opcja A: shared_ptr + shared_from_this
class Sensor : public std::enable_shared_from_this<Sensor> {
    void schedule_read() {
        auto self = shared_from_this();  // ref count +1
        pool_.submit([self] {            // Sensor żyje dopóki task żyje
            self->last_reading_ = 42.0;  // ✅
        });
    }
};
// Tworzenie: auto s = std::make_shared<Sensor>(pool, 1);

// Opcja B: Skopiuj potrzebne dane (nie przechwytuj this)
void schedule_read() {
    int id = sensor_id_;
    pool_.submit([id] {
        std::cout << "Sensor " << id << ": odczyt\n";  // ✅ kopia
    });
}

// Opcja C: Upewnij się że obiekt żyje dłużej niż task
//          (np. future.get() PRZED zniszczeniem obiektu)
```

</details>

---

## Ćwiczenie 11: False sharing

```cpp
struct WorkerCounters {
    std::atomic<long long> tasks_completed;
    std::atomic<long long> total_time_ns;
};

const int NUM_WORKERS = 4;
WorkerCounters counters[NUM_WORKERS];  // tablica obok siebie w pamięci

void worker_fn(int id) {
    for (int i = 0; i < 10'000'000; ++i) {
        auto start = std::chrono::steady_clock::now();

        // Lekka praca
        volatile int x = i * i;
        (void)x;

        auto elapsed = std::chrono::steady_clock::now() - start;
        counters[id].tasks_completed.fetch_add(1);
        counters[id].total_time_ns.fetch_add(
            std::chrono::duration_cast<std::chrono::nanoseconds>(elapsed).count()
        );
    }
}

int main() {
    auto start = std::chrono::steady_clock::now();

    std::vector<std::thread> threads;
    for (int i = 0; i < NUM_WORKERS; ++i)
        threads.emplace_back(worker_fn, i);
    for (auto& t : threads) t.join();

    auto elapsed = std::chrono::steady_clock::now() - start;
    std::cout << "Czas: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count()
              << " ms\n";
}
```

**Pytanie:** Każdy worker operuje **tylko na swoim** `counters[id]`.
Nie ma data race. Ale program jest podejrzanie wolny — znacznie wolniejszy
niż wersja jednowątkowa pomnożona razy 1. Dlaczego?

Podpowiedź: `sizeof(WorkerCounters)` = 16 bytes. Linia cache = 64 bytes.
Ile elementów `counters[]` mieści się w jednej linii cache?

<details>
<summary>Odpowiedź</summary>

**False sharing!**

```
sizeof(WorkerCounters) = 16 bytes (2 × atomic<long long>)
sizeof(cache line) = 64 bytes

counters[0]: bytes 0–15     ┐
counters[1]: bytes 16–31    │ ← JEDNA linia cache (64B)!
counters[2]: bytes 32–47    │
counters[3]: bytes 48–63    ┘
```

Wszystkie 4 elementy tablicy mieszczą się w **jednej linii cache**.
Gdy Worker 0 pisze do `counters[0]`, cała linia cache jest invalidowana
we wszystkich rdzeniach → Worker 1, 2, 3 muszą odczytać ją ponownie z L3/RAM.

Każdy `fetch_add()` powoduje **cache line bouncing** między rdzeniami:

```
CPU 0: fetch_add(counters[0]) → INVALIDATE linia we WSZYSTKICH rdzeniach
CPU 1: fetch_add(counters[1]) → muszę pobrać linię z CPU 0 → INVALIDATE
CPU 2: fetch_add(counters[2]) → muszę pobrać linię z CPU 1 → INVALIDATE
CPU 3: fetch_add(counters[3]) → muszę pobrać linię z CPU 2 → INVALIDATE
... i tak 10'000'000 razy → 10–100× wolniej!
```

**Naprawa:** `alignas(64)` — każdy counter na osobnej linii cache:

```cpp
struct alignas(64) WorkerCounters {  // padding do 64 bytes
    std::atomic<long long> tasks_completed;
    std::atomic<long long> total_time_ns;
    // 48 bytes paddingu (automatycznie przez alignas)
};

// Teraz:
// counters[0]: bytes 0–63      ← osobna linia cache
// counters[1]: bytes 64–127    ← osobna linia cache
// counters[2]: bytes 128–191   ← osobna linia cache
// counters[3]: bytes 192–255   ← osobna linia cache
```

Typowy speedup po naprawie: **5–50×** (zależy od architektury).

</details>

---

## Tabela podsumowująca

Po rozwiązaniu wszystkich ćwiczeń, wypełnij tabelę:

| # | Zagrożenie | Skutek | Wykrywalność | Twoja odpowiedź poprawna? |
|:---:|---|---|:---:|:---:|
| 0 | Wątek per zadanie | | | |
| 1 | Task czeka na subtask | | | |
| 2 | Data race (brak atomics/mutex) | | | |
| 3 | Dangling reference (lambda + &) | | | |
| 4 | Dangling reference (zmienna pętli) | | | |
| 5 | Wyjątek w tasku (bez future) | | | |
| 6 | Kolejność destrukcji memberów | | | |
| 7 | Wyścig z danymi po submit | | | |
| 8 | Niekontrolowany wzrost kolejki | | | |
| 9 | Zagłodzenie krótkich tasków | | | |
| 10 | Dangling `this` | | | |
| 11 | False sharing | | | |

<details>
<summary>Wypełniona tabela</summary>

| # | Zagrożenie | Skutek | Wykrywalność |
|:---:|---|---|:---:|
| 0 | Wątek per zadanie | OOM, thrashing, crash systemu | ✅ Natychmiastowa |
| 1 | Task czeka na subtask | Deadlock (program wisi) | ⚠️ Zależy od obciążenia |
| 2 | Data race (brak atomics/mutex) | Undefined Behavior | ❌ Losowa (TSan pomoże) |
| 3 | Dangling reference (lambda + &) | Use-after-free, UB | ❌ Losowa (ASan pomoże) |
| 4 | Dangling reference (zmienna pętli) | Błędne wartości, UB | ⚠️ Widoczna w output |
| 5 | Wyjątek w tasku (bez future) | `std::terminate()`, cały program pada | ✅ Natychmiastowa |
| 6 | Kolejność destrukcji memberów | Use-after-free, UB | ❌ Losowa, zależy od timingu |
| 7 | Wyścig z danymi po submit | Iterator invalidation, UB | ❌ Losowa |
| 8 | Niekontrolowany wzrost kolejki | Out of memory | ✅ Widoczna (RAM rośnie) |
| 9 | Zagłodzenie krótkich tasków | Katastrofalna latencja | ⚠️ Widoczna w monitoringu |
| 10 | Dangling `this` | Use-after-free, UB | ❌ Losowa |
| 11 | False sharing | 10–100× wolniejsze działanie | ⚠️ Widoczna w benchmarku |

</details>
