# Pula Wątków (Thread Pool)
## Notatki do wykładu/ćwiczeń z programowania współbieżnego w C++

---

## 1. Wprowadzenie — o czym jest ten wzorzec?

**Thread Pool** (pula wątków) to wzorzec projektowy, w którym utrzymujemy
**stałą liczbę wątków** gotowych do wykonywania zadań z **kolejki**.

### Problem: wątek per zadanie

```cpp
// ❌ NAIWNE PODEJŚCIE — nowy wątek na każde zadanie
void handle_requests(std::vector<Request>& requests) {
    std::vector<std::thread> threads;
    for (auto& req : requests) {
        threads.emplace_back([&req] {
            process(req);  // każdy request = nowy wątek
        });
    }
    for (auto& t : threads) t.join();
}
```

Co się stanie gdy `requests.size() == 100'000`?

| Problem | Konsekwencja |
|---------|-------------|
| **Koszt tworzenia wątku** | ~1–100 μs per wątek (syscall `clone`/`CreateThread`) |
| **Zużycie pamięci** | ~1–8 MB stosu per wątek → 100'000 × 8 MB = **800 GB!** |
| **Context switching** | OS musi przełączać się między 100k wątkami → **thrashing** |
| **Brak kontroli** | Nie mamy limitu — możemy wyczerpać zasoby systemu |
| **Cache pollution** | Każdy nowy wątek ma zimny cache → spadek wydajności |

> **Tworzenie wątku jest drogie.** Na typowym Linuxie:
> - `std::thread` construction: ~5–50 μs
> - Alokacja stosu: ~8 KB – 8 MB
> - Rejestracja w schedulerze OS
>
> Jeśli zadanie trwa 1 μs, a tworzenie wątku 10 μs → **90% czasu
> to overhead!**

### Rozwiązanie: pula wątków

```
Podejście "wątek per zadanie":

  Zadanie 1 → [nowy wątek] → wykonaj → zniszcz wątek
  Zadanie 2 → [nowy wątek] → wykonaj → zniszcz wątek
  Zadanie 3 → [nowy wątek] → wykonaj → zniszcz wątek
  ...
  Zadanie N → [nowy wątek] → wykonaj → zniszcz wątek

  Koszt: N × (tworzenie + niszczenie wątku)


Podejście "pula wątków":

  ┌─────────────────────────────────────────────┐
  │              KOLEJKA ZADAŃ                   │
  │  [Zadanie 5] [Zadanie 4] [Zadanie 3] ...   │
  └──────────┬──────────┬──────────┬────────────┘
             │          │          │
         ┌───▼──┐  ┌───▼──┐  ┌───▼──┐
         │ W-1  │  │ W-2  │  │ W-3  │   ← wątki ŻYJĄ cały czas
         │(idle)│  │(busy)│  │(busy)│      biorą zadania z kolejki
         └──────┘  └──────┘  └──────┘

  Koszt tworzenia: 3 × (tworzenie wątku) — JEDNORAZOWO
  Potem: 0 kosztu tworzenia/niszczenia
```

### To jest producent-konsument!

```
  Wątki klienckie (producenci)     Wątki workera (konsumenci)
  ─────────────────────────────    ────────────────────────────
  submit(task_A) ──┐               ┌── Worker 1: task = pop()
  submit(task_B) ──┤               │                execute(task)
  submit(task_C) ──┤  ┌────────┐  ├── Worker 2: task = pop()
  submit(task_D) ──┼─►│KOLEJKA │──┤                execute(task)
  submit(task_E) ──┤  │ ZADAŃ  │  ├── Worker 3: task = pop()
  submit(task_F) ──┘  └────────┘  │                execute(task)
                                  └── Worker 4: (czeka na zadanie...)
                   ▲                              ▲
                   │                              │
              BoundedBuffer              pop() z condition_variable
              z zajęć 1!                 (śpi gdy kolejka pusta)
```

> **Kluczowa obserwacja:** Thread Pool to bezpośrednie zastosowanie wzorca
> **producent-konsument** — wątki klienckie produkują zadania, wątki
> workerów je konsumują. Możemy reużyć `BoundedBuffer` z pierwszych zajęć!

### Przykłady z rzeczywistego świata

| System | Producenci | Kolejka | Konsumenci (pool) |
|--------|-----------|---------|-------------------|
| Serwer HTTP (nginx, Apache) | Wątek akceptujący połączenia | Kolejka requestów | Wątki workerów przetwarzające HTTP |
| Baza danych (PostgreSQL) | Klienty wysyłające zapytania | Kolejka query | Worker processes |
| Silnik gry (Unreal, Unity) | Main thread zlecający zadania | Task queue | Worker threads (physics, AI, rendering) |
| Kompilator (make -j8) | Pliki do kompilacji | Kolejka plików | 8 procesów kompilatora |
| Java `ExecutorService` | `executor.submit(task)` | `BlockingQueue` | Wątki z puli |
| Go goroutines | `go func()` | Run queue | OS threads (GOMAXPROCS) |
| `std::async` (C++) | Wywołanie `std::async` | Impl.-defined | Impl.-defined thread pool |

---

## 2. Narzędzia z biblioteki standardowej C++

Zanim zaimplementujemy thread pool, poznajmy narzędzia do **zwracania wyników**
z asynchronicznych zadań.

### 2.1 `std::future` i `std::promise` — kanał komunikacji

```cpp
#include <future>
#include <thread>
#include <iostream>

int main() {
    // promise + future = jednorazowy kanał komunikacji między wątkami
    //
    // promise → pisze wynik (producent)
    // future  → czyta wynik (konsument)

    std::promise<int> promise;
    std::future<int> future = promise.get_future();

    // Wątek obliczający
    std::thread worker([&promise] {
        int wynik = 42 * 42;
        promise.set_value(wynik);  // "dostarczam wynik"
    });

    // Wątek główny czeka na wynik
    int wynik = future.get();  // BLOKUJE aż set_value() zostanie wywołane
    std::cout << "Wynik: " << wynik << "\n";  // 1764

    worker.join();
}
```

### Wizualizacja przepływu

```
Wątek główny                    Wątek worker
──────────────                  ─────────────
promise<int> p;
future<int> f = p.get_future();
                                // ... obliczenia ...
f.get()  ←── BLOKUJE ──────── p.set_value(1764);
         ←── ODBLOKOWANY ──┘
wynik = 1764 ✅
```

### 2.2 `std::future` — obsługa wyjątków

```cpp
std::promise<int> promise;
std::future<int> future = promise.get_future();

std::thread worker([&promise] {
    try {
        // Symulacja błędu
        throw std::runtime_error("Coś poszło nie tak!");
    } catch (...) {
        // Przekazujemy wyjątek przez promise!
        promise.set_exception(std::current_exception());
    }
});

try {
    int wynik = future.get();  // RZUCI wyjątek z set_exception!
} catch (const std::exception& e) {
    std::cout << "Błąd: " << e.what() << "\n";
}

worker.join();
```

> **Kluczowe:** `future.get()` re-rzuca wyjątek ustawiony przez
> `promise.set_exception()`. Dzięki temu wyjątki **przelatują
> między wątkami**.

### 2.3 `std::packaged_task` — opakowanie funkcji w promise

`std::packaged_task` automatycznie tworzy `promise` i ustawia wynik/wyjątek:

```cpp
#include <future>
#include <iostream>

int oblicz(int x, int y) {
    return x * y;
}

int main() {
    // packaged_task = function wrapper + promise
    std::packaged_task<int(int, int)> task(oblicz);

    // Wyciągamy future ZANIM wywołamy task
    std::future<int> future = task.get_future();

    // task można wywołać w innym wątku:
    std::thread worker(std::move(task), 6, 7);

    int wynik = future.get();  // czekamy na wynik
    std::cout << "6 × 7 = " << wynik << "\n";  // 42

    worker.join();
}
```

### Porównanie narzędzi

| Narzędzie | Co robi | Kiedy używać |
|-----------|---------|-------------|
| `std::promise<T>` + `std::future<T>` | Ręczny kanał: `set_value` → `get` | Gdy sam kontrolujesz kiedy wynik jest gotowy |
| `std::packaged_task<R(Args...)>` | Opakowuje callable → automatyczny promise | Gdy masz funkcję i chcesz ją zlecić wątkowi |
| `std::async(policy, f, args...)` | Tworzy task + (opcjonalnie) wątek | Szybkie fire-and-forget |

### 2.4 `std::async` — najprostsze, ale ograniczone

```cpp
#include <future>
#include <iostream>

int oblicz_ciezko(int x) {
    std::this_thread::sleep_for(std::chrono::seconds(2));
    return x * x;
}

int main() {
    // Wariant 1: std::launch::async — GWARANTUJE nowy wątek
    auto future1 = std::async(std::launch::async, oblicz_ciezko, 42);

    // Wariant 2: std::launch::deferred — NIE tworzy wątku!
    //            Obliczenie nastąpi dopiero przy .get()
    auto future2 = std::async(std::launch::deferred, oblicz_ciezko, 7);

    // Wariant 3: domyślny — implementacja DECYDUJE
    auto future3 = std::async(oblicz_ciezko, 13);

    std::cout << future1.get() << "\n";  // 1764 (po ~2s)
    std::cout << future2.get() << "\n";  // 49 (obliczone TERAZ, leniwie)
    std::cout << future3.get() << "\n";  // 169 (kto wie kiedy?)
}
```

### Problemy z `std::async`

| Problem | Opis |
|---------|------|
| **Brak puli** | `std::launch::async` tworzy **nowy wątek** per wywołanie — nie reużywa! |
| **Deferred = brak równoległości** | `std::launch::deferred` to leniwe obliczenie w tym samym wątku |
| **Domyślna polityka jest nieokreślona** | Implementacja może wybrać `deferred` → 0 równoległości |
| **Destruktor future BLOKUJE** | Jeśli `future` z `std::async` wyjdzie z scope → `join()` |
| **Brak kontroli** | Nie możesz ustalić max wątków, priorytetów, CPU affinity |

```cpp
// ⚠️ PUŁAPKA: destruktor future z std::async BLOKUJE!
{
    auto f = std::async(std::launch::async, [] {
        std::this_thread::sleep_for(std::chrono::seconds(10));
    });
    // f wychodzi z scope → ~future() CZEKA 10 sekund!
}
// Dopiero tu kontynuujemy — po 10 sekundach!
```

> **Wniosek:** `std::async` jest wygodne do prototypów, ale w produkcji
> potrzebujesz **Thread Pool** z kontrolą nad liczbą wątków i kolejką.

---

## 3. Implementacja Thread Pool — krok po kroku

### 3.1 Wersja minimalna — `submit()` bez wyniku

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <vector>
#include <functional>
#include <atomic>

class ThreadPool {
    std::vector<std::thread> workers_;

    std::queue<std::function<void()>> tasks_;
    std::mutex mutex_;
    std::condition_variable cv_;
    bool shutdown_ = false;

public:
    explicit ThreadPool(size_t num_threads) {
        for (size_t i = 0; i < num_threads; ++i) {
            workers_.emplace_back([this] {
                worker_loop();
            });
        }
    }

    ~ThreadPool() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            shutdown_ = true;
        }
        cv_.notify_all();  // obudź WSZYSTKICH workerów
        for (auto& w : workers_) {
            w.join();
        }
    }

    // Dodaje zadanie do kolejki
    void submit(std::function<void()> task) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (shutdown_) {
                throw std::runtime_error("submit() na zamkniętym poolu!");
            }
            tasks_.push(std::move(task));
        }
        cv_.notify_one();  // obudź jednego workera
    }

private:
    void worker_loop() {
        while (true) {
            std::function<void()> task;
            {
                std::unique_lock<std::mutex> lock(mutex_);

                // Czekaj na: (zadanie w kolejce) LUB (shutdown)
                cv_.wait(lock, [this] {
                    return !tasks_.empty() || shutdown_;
                });

                // Shutdown + pusta kolejka → kończymy
                if (shutdown_ && tasks_.empty()) {
                    return;
                }

                task = std::move(tasks_.front());
                tasks_.pop();
            }
            // Wykonaj POZA mutexem!
            task();
        }
    }
};
```

### Analiza: dlaczego to jest producent-konsument?

```
submit()     = producent    → wkłada task do kolejki
worker_loop  = konsument    → wyjmuje task z kolejki

mutex_  = chroni kolejkę tasks_
cv_     = budzenie workerów gdy pojawi się zadanie
          (= cv_not_empty_ z BoundedBuffer!)

shutdown_ = graceful shutdown
            (= close() z BoundedBufferWithShutdown!)
```

### Użycie

```cpp
int main() {
    ThreadPool pool(4);  // 4 wątki workerów

    for (int i = 0; i < 20; ++i) {
        pool.submit([i] {
            std::cout << "Zadanie " << i
                      << " na wątku " << std::this_thread::get_id() << "\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        });
    }

    // ~ThreadPool() czeka aż wszystkie zadania się zakończą
}
```

```
Wyjście (przykładowe — kolejność niedeterministyczna):

  Zadanie 0 na wątku 140234567891200
  Zadanie 1 na wątku 140234567886848
  Zadanie 2 na wątku 140234567882496
  Zadanie 3 na wątku 140234567878144
  Zadanie 4 na wątku 140234567891200   ← REUŻYCIE wątku!
  ...
```

> **Zwróć uwagę:** Wątek `140234567891200` wykonał zadanie 0, a potem zadanie 4.
> Nie tworzymy nowych wątków — workerzy biorą kolejne zadania z kolejki.

---

### 3.2 Wersja z wynikami — `submit()` zwracające `std::future`

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <vector>
#include <functional>
#include <future>
#include <type_traits>

class ThreadPool {
    std::vector<std::thread> workers_;

    std::queue<std::function<void()>> tasks_;
    std::mutex mutex_;
    std::condition_variable cv_;
    bool shutdown_ = false;

public:
    explicit ThreadPool(size_t num_threads) {
        for (size_t i = 0; i < num_threads; ++i) {
            workers_.emplace_back([this] { worker_loop(); });
        }
    }

    ~ThreadPool() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            shutdown_ = true;
        }
        cv_.notify_all();
        for (auto& w : workers_) w.join();
    }

    // ─── Wersja zwracająca future ───
    //
    // Magia szablonów:
    //   F        = typ callable (lambda, function pointer, functor)
    //   Args...  = typy argumentów
    //   R        = typ zwracany przez F(Args...)
    //
    // Zwraca: std::future<R> — "obietnica" wyniku
    //
    template<typename F, typename... Args>
    auto submit(F&& f, Args&&... args)
        -> std::future<std::invoke_result_t<F, Args...>>
    {
        using R = std::invoke_result_t<F, Args...>;

        // 1. Opakowujemy f(args...) w packaged_task
        auto task = std::make_shared<std::packaged_task<R()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );

        // 2. Wyciągamy future ZANIM task trafi do kolejki
        std::future<R> future = task->get_future();

        // 3. Wkładamy do kolejki jako std::function<void()>
        //    (packaged_task nie jest copyable — stąd shared_ptr)
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (shutdown_) {
                throw std::runtime_error("submit() na zamkniętym poolu!");
            }
            tasks_.push([task]() { (*task)(); });
        }
        cv_.notify_one();

        return future;
    }

private:
    void worker_loop() {
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
    }
};
```

### Użycie z wynikami

```cpp
int main() {
    ThreadPool pool(4);

    // --- Zadanie zwracające wynik ---
    auto future1 = pool.submit([] {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        return 42;
    });

    // --- Zadanie z argumentami ---
    auto future2 = pool.submit([](int a, int b) {
        return a + b;
    }, 100, 200);

    // --- Zadanie rzucające wyjątek ---
    auto future3 = pool.submit([] () -> int {
        throw std::runtime_error("Ups!");
    });

    std::cout << "Wynik 1: " << future1.get() << "\n";  // 42
    std::cout << "Wynik 2: " << future2.get() << "\n";  // 300

    try {
        future3.get();  // rzuci std::runtime_error!
    } catch (const std::exception& e) {
        std::cout << "Wyjątek: " << e.what() << "\n";   // "Ups!"
    }
}
```

### Przepływ danych — wizualizacja

```
Klient                    Kolejka                 Worker
───────                   ───────                 ──────

submit(f, args...)
   │
   ├─ packaged_task<R()> task = bind(f, args)
   ├─ future<R> = task.get_future()
   ├─ tasks_.push( [task]{ (*task)(); } )
   │                  │
   │                  ▼
   │            ┌───────────┐
   │            │ task( )   │
   │            └─────┬─────┘
   │                  │
   │                  ▼
   │            worker pobiera task
   │            wykonuje (*task)()
   │                  │
   │                  ├─ sukces → task ustawia promise.set_value(R)
   │                  └─ wyjątek → task ustawia promise.set_exception(e)
   │
   ▼
future.get()  ← BLOKUJE aż worker ustawi wynik
   │
   ├─ wynik dostępny → zwraca R
   └─ wyjątek → rzuca ponownie
```

---

### 3.3 Anatomia `submit()` — krok po kroku

Rozbijmy `submit()` na części, bo to **najtrudniejsza funkcja** w całym poolie:

```cpp
template<typename F, typename... Args>
auto submit(F&& f, Args&&... args)
    -> std::future<std::invoke_result_t<F, Args...>>
```

**Krok 1: Dedukujemy typ wyniku**

```cpp
using R = std::invoke_result_t<F, Args...>;
// Jeśli f = [](int a, int b) -> double { return a + b; }
// i args = (3, 4)
// to R = double
```

**Krok 2: Tworzymy `packaged_task`**

```cpp
auto task = std::make_shared<std::packaged_task<R()>>(
    std::bind(std::forward<F>(f), std::forward<Args>(args)...)
);
```

Dlaczego `shared_ptr`?
- `std::packaged_task` jest **move-only** (nie można kopiować)
- `std::function<void()>` wymaga **copyable** callable
- `shared_ptr` daje nam **copyable wrapper** wokół move-only obiektu

Dlaczego `std::bind`?
- `packaged_task<R()>` ma sygnaturę **bez argumentów** (`R()`)
- `std::bind(f, args...)` tworzy callable bez argumentów (argumenty "wpieczętowane")

**Krok 3: Wyciągamy `future`**

```cpp
std::future<R> future = task->get_future();
// MUSI być PRZED wrzuceniem task do kolejki!
// Bo worker może wykonać task ZANIM my zrobimy get_future()
```

**Krok 4: Wkładamy wrapper do kolejki**

```cpp
tasks_.push([task]() { (*task)(); });
// Lambda przechwytuje shared_ptr<packaged_task> przez KOPIĘ
// Gdy worker wywoła tę lambdę → (*task)() wykona oryginalną funkcję
// → packaged_task automatycznie ustawi wynik w promise
```

---

## 4. Warianty i rozszerzenia

### 4.1 Thread Pool z ograniczoną kolejką (backpressure)

Dotychczasowa kolejka jest **nieograniczona** — jeśli producenci są szybsi
niż konsumenci, pamięć rośnie bez limitu. Rozwiązanie: `BoundedBuffer`!

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <vector>
#include <functional>
#include <future>
#include <cassert>

template<typename T>
class BoundedBuffer {
    // Reużywamy implementację z zajęć "Producent-Konsument"!
    std::queue<T> queue_;
    const size_t max_size_;
    std::mutex mutex_;
    std::condition_variable cv_not_full_;
    std::condition_variable cv_not_empty_;
    bool shutdown_ = false;

public:
    explicit BoundedBuffer(size_t max_size) : max_size_(max_size) {
        assert(max_size > 0);
    }

    // Zwraca false jeśli shutdown
    bool push(T item) {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_not_full_.wait(lock, [this] {
            return queue_.size() < max_size_ || shutdown_;
        });
        if (shutdown_) return false;
        queue_.push(std::move(item));
        cv_not_empty_.notify_one();
        return true;
    }

    // Zwraca nullopt jeśli shutdown i pusta
    std::optional<T> pop() {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_not_empty_.wait(lock, [this] {
            return !queue_.empty() || shutdown_;
        });
        if (queue_.empty()) return std::nullopt;
        T item = std::move(queue_.front());
        queue_.pop();
        cv_not_full_.notify_one();
        return item;
    }

    void close() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            shutdown_ = true;
        }
        cv_not_full_.notify_all();
        cv_not_empty_.notify_all();
    }
};

class BoundedThreadPool {
    std::vector<std::thread> workers_;
    BoundedBuffer<std::function<void()>> tasks_;

public:
    BoundedThreadPool(size_t num_threads, size_t max_pending)
        : tasks_(max_pending)
    {
        for (size_t i = 0; i < num_threads; ++i) {
            workers_.emplace_back([this] {
                while (auto task = tasks_.pop()) {
                    (*task)();  // wykonaj zadanie
                }
                // pop() zwrócił nullopt → shutdown
            });
        }
    }

    ~BoundedThreadPool() {
        tasks_.close();
        for (auto& w : workers_) w.join();
    }

    // BLOKUJE jeśli kolejka pełna! (backpressure)
    void submit(std::function<void()> task) {
        if (!tasks_.push(std::move(task))) {
            throw std::runtime_error("Pool zamknięty!");
        }
    }

    // Wersja z future — analogicznie do 3.2
    template<typename F, typename... Args>
    auto submit_with_result(F&& f, Args&&... args)
        -> std::future<std::invoke_result_t<F, Args...>>
    {
        using R = std::invoke_result_t<F, Args...>;
        auto task = std::make_shared<std::packaged_task<R()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );
        auto future = task->get_future();
        submit([task]() { (*task)(); });
        return future;
    }
};
```

### Wizualizacja backpressure

```
Producent szybszy niż konsumenci:

  submit(A) ✅ → [A]
  submit(B) ✅ → [A][B]
  submit(C) ✅ → [A][B][C]          ← kolejka pełna (max_pending=3)
  submit(D) → BLOKUJE ──────────── worker bierze A → [B][C]
           → ODBLOKOWANY → [B][C][D] ✅

Bez backpressure:
  submit() nigdy nie blokuje → kolejka rośnie → OOM ☠️

Z backpressure:
  submit() BLOKUJE gdy kolejka pełna → producent zwalnia → system stabilny ✅
```

> **Analogia:** To jak kolejka do kasy w sklepie. Gdy jest za długa,
> klienci (producenci) czekają na wejściu. To **chroni system** przed
> przeciążeniem.

### 4.2 Thread Pool z priorytetami

```cpp
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <thread>
#include <future>

class PriorityThreadPool {
    struct Task {
        int priority;  // wyższy = ważniejszy
        std::function<void()> func;

        // Odwrócona kolejność — priority_queue to max-heap
        bool operator<(const Task& other) const {
            return priority < other.priority;
        }
    };

    std::vector<std::thread> workers_;
    std::priority_queue<Task> tasks_;
    std::mutex mutex_;
    std::condition_variable cv_;
    bool shutdown_ = false;

public:
    explicit PriorityThreadPool(size_t num_threads) {
        for (size_t i = 0; i < num_threads; ++i) {
            workers_.emplace_back([this] { worker_loop(); });
        }
    }

    ~PriorityThreadPool() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            shutdown_ = true;
        }
        cv_.notify_all();
        for (auto& w : workers_) w.join();
    }

    void submit(std::function<void()> func, int priority = 0) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            tasks_.push({priority, std::move(func)});
        }
        cv_.notify_one();
    }

private:
    void worker_loop() {
        while (true) {
            std::function<void()> task;
            {
                std::unique_lock<std::mutex> lock(mutex_);
                cv_.wait(lock, [this] {
                    return !tasks_.empty() || shutdown_;
                });
                if (shutdown_ && tasks_.empty()) return;

                task = std::move(const_cast<Task&>(tasks_.top()).func);
                tasks_.pop();
            }
            task();
        }
    }
};
```

```cpp
// Użycie:
PriorityThreadPool pool(4);

pool.submit([] { /* ... */ }, 1);   // niski priorytet
pool.submit([] { /* ... */ }, 10);  // wysoki priorytet  ← wykona się PIERWSZY
pool.submit([] { /* ... */ }, 5);   // średni priorytet
```

> **⚠️ Zagłodzenie!** Zadania o niskim priorytecie mogą nigdy nie zostać
> wykonane jeśli ciągle napływają zadania o wysokim priorytecie.
> Rozwiązanie: **aging** — priorytet rośnie z czasem oczekiwania.

### 4.3 Thread Pool z liczbą wątków = `std::thread::hardware_concurrency()`

```cpp
class AutoThreadPool : public ThreadPool {
public:
    AutoThreadPool()
        : ThreadPool(std::thread::hardware_concurrency()) {}

    // hardware_concurrency() zwraca liczbę rdzeni:
    //   4-rdzeniowy CPU → 4
    //   8-rdzeniowy z HT → 16
    //   Nie można ustalić → 0 (!) → zabezpiecz się!
};

// Bezpieczna wersja:
size_t default_threads() {
    size_t n = std::thread::hardware_concurrency();
    return (n > 0) ? n : 4;  // fallback: 4
}
```

### 4.4 Kiedy ile wątków?

| Typ zadań | Rekomendacja | Dlaczego |
|-----------|-------------|----------|
| **CPU-bound** (obliczenia) | `hardware_concurrency()` | Więcej wątków = context switching overhead |
| **I/O-bound** (sieć, dysk) | 2× – 10× `hardware_concurrency()` | Wątki i tak śpią czekając na I/O |
| **Mieszane** | Zmierz! | Optymalny punkt zależy od proporcji CPU/IO |

```
CPU-bound (4 rdzenie):

  4 wątki:  ████████████████████  throughput ≈ 100%
  8 wątków: ████████████████████  throughput ≈ 98% (overhead CS)
  64 wątki: █████████████         throughput ≈ 60% (dużo CS)

I/O-bound (4 rdzenie, 50% czasu = czekanie na I/O):

  4 wątki:  ██████████            throughput ≈ 50%
  8 wątków: ████████████████████  throughput ≈ 100%  ← 2× hardware_concurrency
  16 wątków:████████████████████  throughput ≈ 100% (diminishing returns)
```

---

## 5. Zagrożenia i pułapki

### 5.1 Deadlock w Thread Pool — zadanie czeka na inne zadanie

```cpp
ThreadPool pool(2);  // TYLKO 2 wątki!

auto future = pool.submit([&pool] {
    // Zlecamy podzadanie na TEN SAM pool:
    auto inner = pool.submit([] {
        return 42;
    });
    return inner.get();  // ❌ CZEKAMY na wynik podzadania
});

// Scenariusz:
//   Worker 1: wykonuje outer task → submit(inner) → get() → CZEKA
//   Worker 2: wykonuje inny task → submit(inner) → get() → CZEKA
//   Nikt nie wykona inner task → DEADLOCK!
```

```
Worker 1: outer_task → inner.get() → 💤 czeka na inner
Worker 2: outer_task → inner.get() → 💤 czeka na inner
Kolejka:  [inner_task, inner_task]  ← NIKT ICH NIE WYKONA!

        ┌─► Worker 1 czeka na inner_task w kolejce
        │   Worker 2 czeka na inner_task w kolejce
        │   Kolejka czeka na wolnego workera
        └── Brak wolnych workerów ← DEADLOCK ☠️
```

> **Zasada:** Nigdy nie rób `future.get()` wewnątrz zadania na tym samym poolu
> (chyba że pool potrafi **dynamicznie** dodawać wątki — patrz work stealing).

### Rozwiązania

| Rozwiązanie | Opis |
|-------------|------|
| **Osobne poole** | Outer tasks na pool A, inner tasks na pool B |
| **Więcej wątków** | Ale ile wystarczy? Trudno przewidzieć głębokość rekursji |
| **Work stealing** | Worker „kradnie" zadanie z kolejki innego workera |
| **Coroutines (C++20)** | `co_await` zamiast `future.get()` — nie blokuje wątku |

### 5.2 Wyjątek w zadaniu — co się dzieje?

```cpp
pool.submit([] {
    throw std::runtime_error("Boom!");
    // Co się stanie?
});
```

**Wersja `submit(function<void()>)`** (bez future):
- Wyjątek leci z `task()` w `worker_loop()`
- Wyjątek **nie jest łapany** → `std::terminate()` → **cały program pada!**

**Rozwiązanie:** Łap wyjątki w worker_loop:

```cpp
void worker_loop() {
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
        try {
            task();
        } catch (const std::exception& e) {
            std::cerr << "Wyjątek w zadaniu: " << e.what() << "\n";
        } catch (...) {
            std::cerr << "Nieznany wyjątek w zadaniu!\n";
        }
    }
}
```

**Wersja `submit()` z `future`:**
- `packaged_task` **automatycznie** łapie wyjątek i przekazuje przez `future`
- Wyjątek jest „odłożony" do momentu `future.get()`
- Worker **nie pada** ✅

### 5.3 Dangling references — lambda przechwytuje referencję

```cpp
void bad_example(ThreadPool& pool) {
    int local_var = 42;

    pool.submit([&local_var] {
        // ❌ local_var może już nie istnieć gdy task się wykona!
        std::cout << local_var << "\n";  // 💥 UB!
    });

}   // ← local_var ZNISZCZONE, a task może jeszcze nie wystartował!
```

**Rozwiązanie:** Przechwytuj przez **kopię**, nie referencję:

```cpp
void good_example(ThreadPool& pool) {
    int local_var = 42;

    pool.submit([local_var] {       // kopia!
        std::cout << local_var << "\n";  // ✅ OK
    });

    // Albo użyj shared_ptr dla ciężkich obiektów:
    auto data = std::make_shared<std::vector<int>>(1000000, 0);
    pool.submit([data] {            // shared_ptr przez kopię
        process(*data);              // ✅ dane żyją dopóki task żyje
    });
}
```

### 5.4 Thread pool destruction order

```cpp
class Server {
    ThreadPool pool_;            // (1) tworzony PIERWSZY
    std::shared_ptr<Database> db_;  // (2) tworzony DRUGI

public:
    Server() : pool_(4), db_(std::make_shared<Database>()) {
        pool_.submit([this] {
            db_->query("SELECT ...");   // ❌ db_ może być zniszczone PRZED pool_!
        });
    }

    // Destruktory w ODWROTNEJ kolejności:
    //   (2) ~db_  ← PIERWSZE! Baza zniszczona!
    //   (1) ~pool_ ← join()... ale taski mogą jeszcze używać db_! 💥
};
```

**Rozwiązanie:** Pool jako **ostatni** member (niszczony PIERWSZY):

```cpp
class Server {
    std::shared_ptr<Database> db_;  // (1) tworzony pierwszy, niszczony DRUGI
    ThreadPool pool_;               // (2) tworzony drugi, niszczony PIERWSZY
    // ~pool_() → join() → wszystkie taski kończą → potem ~db_() ✅
};
```

> **Reguła:** Thread pool powinien być **ostatnim** memberem klasy
> (lub jawnie shutdown'owany w destruktorze PRZED niszczeniem zależności).

---

## 6. Wzorce użycia Thread Pool

### 6.1 Parallel map — przetwarzanie kolekcji równolegle

```cpp
#include <vector>
#include <future>
#include <algorithm>

template<typename T, typename F>
std::vector<std::invoke_result_t<F, const T&>>
parallel_map(ThreadPool& pool, const std::vector<T>& input, F&& func) {
    using R = std::invoke_result_t<F, const T&>;

    std::vector<std::future<R>> futures;
    futures.reserve(input.size());

    for (const auto& item : input) {
        futures.push_back(pool.submit(func, item));
    }

    std::vector<R> results;
    results.reserve(input.size());

    for (auto& f : futures) {
        results.push_back(f.get());
    }

    return results;
}

// Użycie:
ThreadPool pool(4);
std::vector<int> dane = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

auto kwadraty = parallel_map(pool, dane, [](int x) {
    return x * x;
});
// kwadraty = {1, 4, 9, 16, 25, 36, 49, 64, 81, 100}
```

### 6.2 Parallel reduce — agregacja równoległa

```cpp
template<typename T, typename F>
T parallel_reduce(ThreadPool& pool, const std::vector<T>& input,
                  T init, F&& combine, size_t chunk_size = 1000)
{
    if (input.size() <= chunk_size) {
        T result = init;
        for (const auto& x : input) result = combine(result, x);
        return result;
    }

    // Podziel na chunki
    std::vector<std::future<T>> futures;
    for (size_t i = 0; i < input.size(); i += chunk_size) {
        size_t end = std::min(i + chunk_size, input.size());
        futures.push_back(pool.submit(
            [&input, &combine, init, i, end] {
                T local_result = init;
                for (size_t j = i; j < end; ++j) {
                    local_result = combine(local_result, input[j]);
                }
                return local_result;
            }
        ));
    }

    // Zbierz wyniki
    T result = init;
    for (auto& f : futures) {
        result = combine(result, f.get());
    }
    return result;
}

// Użycie:
std::vector<int> dane(1'000'000);
std::iota(dane.begin(), dane.end(), 1);  // {1, 2, ..., 1000000}

long long suma = parallel_reduce(pool, dane, 0LL,
    [](long long a, int b) { return a + b; });
// suma = 500000500000
```

### 6.3 Parallel for_each — ogólna pętla równoległa

```cpp
template<typename Iter, typename F>
void parallel_for_each(ThreadPool& pool, Iter begin, Iter end, F&& func,
                       size_t chunk_size = 100)
{
    std::vector<std::future<void>> futures;

    for (auto it = begin; it < end; ) {
        auto chunk_end = std::min(it + chunk_size,
                                  static_cast<Iter>(end));
        futures.push_back(pool.submit(
            [it, chunk_end, &func] {
                for (auto i = it; i < chunk_end; ++i) {
                    func(*i);
                }
            }
        ));
        it = chunk_end;
    }

    for (auto& f : futures) f.get();
}

// Użycie:
std::vector<double> data(100000, 1.0);

parallel_for_each(pool, data.begin(), data.end(),
    [](double& x) { x = std::sin(x) * std::cos(x); });
```

### 6.4 Pipeline wieloetapowy na Thread Poolach

```cpp
// Etap 1: Generowanie → Etap 2: Transformacja → Etap 3: Zapis
//
//  Pool nie musi być osobny na etap — jeden pool, różne zadania!

template<typename T>
class Pipeline {
    ThreadPool& pool_;
    BoundedBuffer<T> buffer1_;
    BoundedBuffer<T> buffer2_;

public:
    Pipeline(ThreadPool& pool, size_t buf_size)
        : pool_(pool), buffer1_(buf_size), buffer2_(buf_size) {}

    // Uruchom pipeline: generator → transform → consumer
    template<typename Gen, typename Trans, typename Cons>
    void run(Gen generator, Trans transform, Cons consumer,
             int num_items)
    {
        // Etap 1: Generator
        auto f1 = pool_.submit([&, num_items] {
            for (int i = 0; i < num_items; ++i) {
                T item = generator(i);
                buffer1_.push(item);
            }
            buffer1_.close();
        });

        // Etap 2: Transformer
        auto f2 = pool_.submit([&] {
            while (auto item = buffer1_.pop()) {
                T transformed = transform(*item);
                buffer2_.push(transformed);
            }
            buffer2_.close();
        });

        // Etap 3: Consumer
        auto f3 = pool_.submit([&] {
            while (auto item = buffer2_.pop()) {
                consumer(*item);
            }
        });

        f1.get();
        f2.get();
        f3.get();
    }
};

// Użycie:
ThreadPool pool(4);
Pipeline<int> pipe(pool, 100);

pipe.run(
    [](int i) { return i; },                    // generator
    [](int x) { return x * x; },                // transform
    [](int x) { std::cout << x << " "; },       // consumer
    20
);
```

---

## 7. Porównanie podejść — tabela podsumowująca

| Podejście | Reużycie wątków | Wyniki | Backpressure | Priorytety | Złożoność |
|-----------|:---:|:---:|:---:|:---:|:---:|
| `std::thread` per zadanie | ❌ | ❌ | ❌ | ❌ | ⭐ |
| `std::async` | ⚠️ impl. defined | ✅ `future` | ❌ | ❌ | ⭐ |
| ThreadPool (basic) | ✅ | ❌ | ❌ | ❌ | ⭐⭐ |
| ThreadPool + `future` | ✅ | ✅ | ❌ | ❌ | ⭐⭐⭐ |
| BoundedThreadPool | ✅ | ✅ | ✅ | ❌ | ⭐⭐⭐ |
| PriorityThreadPool | ✅ | ⚠️ | ❌ | ✅ | ⭐⭐⭐⭐ |
| Work-stealing pool | ✅ | ✅ | ✅ | ❌ | ⭐⭐⭐⭐⭐ |

---

## 8. Benchmark: Thread per task vs Thread Pool vs `std::async`

```cpp
#include <iostream>
#include <thread>
#include <future>
#include <vector>
#include <chrono>
#include <functional>
#include <numeric>

// ThreadPool — wersja z sekcji 3.2
// (wklej tutaj swoją implementację)

void lekkie_zadanie() {
    volatile int sum = 0;
    for (int i = 0; i < 100; ++i) sum += i;
}

template<typename F>
double measure_ms(F&& func) {
    auto start = std::chrono::steady_clock::now();
    func();
    auto end = std::chrono::steady_clock::now();
    return std::chrono::duration<double, std::milli>(end - start).count();
}

int main() {
    const int N = 10'000;
    const int THREADS = std::thread::hardware_concurrency();

    // --- Wariant 1: Thread per task ---
    double t1 = measure_ms([&] {
        std::vector<std::thread> threads;
        threads.reserve(N);
        for (int i = 0; i < N; ++i)
            threads.emplace_back(lekkie_zadanie);
        for (auto& t : threads) t.join();
    });

    // --- Wariant 2: std::async ---
    double t2 = measure_ms([&] {
        std::vector<std::future<void>> futures;
        futures.reserve(N);
        for (int i = 0; i < N; ++i)
            futures.push_back(std::async(std::launch::async, lekkie_zadanie));
        for (auto& f : futures) f.get();
    });

    // --- Wariant 3: Thread Pool ---
    double t3 = measure_ms([&] {
        ThreadPool pool(THREADS);
        std::vector<std::future<void>> futures;
        futures.reserve(N);
        for (int i = 0; i < N; ++i)
            futures.push_back(pool.submit(lekkie_zadanie));
        for (auto& f : futures) f.get();
    });

    std::cout << "=== " << N << " lekkich zadań ===\n";
    std::cout << "Thread per task:  " << t1 << " ms\n";
    std::cout << "std::async:       " << t2 << " ms\n";
    std::cout << "Thread Pool (" << THREADS << "t): " << t3 << " ms\n";
    std::cout << "\nSpeedup Pool vs Thread-per-task: "
              << t1 / t3 << "x\n";
}
```

### Oczekiwane wyniki (przybliżone, 8-rdzeniowy CPU)

```
=== 10000 lekkich zadań ===
Thread per task:  1250.3 ms     ← tworzenie/niszczenie 10k wątków
std::async:        850.7 ms     ← podobnie, zależy od implementacji
Thread Pool (8t):   12.4 ms     ← 100× szybciej!

Speedup Pool vs Thread-per-task: 100.8x
```

| Podejście | Co trwa najdłużej | Czas na 10k zadań |
|-----------|-------------------|:-:|
| Thread per task | Tworzenie 10000 wątków | ~1000 ms |
| `std::async` | Tworzenie wątków (ale impl. może poolować) | ~500–1000 ms |
| Thread Pool (8t) | Samo wykonanie zadań (8 wątków reużywanych) | ~10 ms |

---

## 9. Zaawansowane: Work Stealing (informacyjnie)

### Problem z jedną kolejką

```
Kolejka zadań: [T1] [T2] [T3] [T4] [T5] [T6] [T7] [T8]
                              │
                    ┌─────────┼─────────┐
                    │         │         │
                 Worker 1  Worker 2  Worker 3

Wszyscy workerzy rywalizują o JEDEN mutex
→ Contention rośnie z liczbą wątków!
→ Na 64 rdzeniach — dramatyczny spadek wydajności
```

### Rozwiązanie: każdy worker ma SWOJĄ kolejkę

```
Worker 1: [T1] [T4] [T7]     ← swoja lokalna kolejka
Worker 2: [T2] [T5] [T8]     ← swoja lokalna kolejka
Worker 3: [T3] [T6]          ← swoja lokalna kolejka

Worker 3 skończył swoje zadania i jest idle...
→ „KRADNIE" zadanie z kolejki Workera 1!

Worker 1: [T4] [T7]          ← T1 skradziony przez Worker 3
Worker 2: [T5] [T8]
Worker 3: [T1]                ← skradziony task
```

### Schemat work stealing

```
submit(task):
  → Wrzuć do kolejki BIEŻĄCEGO wątku (lub losowego)

worker_loop():
  while (true) {
      task = my_queue.pop_local();          // 1. sprawdź SWOJĄ kolejkę
      if (!task) task = steal_from_other();  // 2. kradnij od innego
      if (!task) wait_or_sleep();            // 3. czekaj
      execute(task);
  }
```

### Porównanie

| Aspekt | Jedna kolejka | Work stealing |
|--------|:---:|:---:|
| Contention na mutex | ❌ Wysoka (1 mutex) | ✅ Niska (N kolejek) |
| Load balancing | ✅ Naturalny | ✅ Przez kradzież |
| Lokalność cache | ❌ Słaba | ✅ Lokalne zadania = gorący cache |
| Złożoność implementacji | ⭐⭐ | ⭐⭐⭐⭐⭐ |
| Implementacje | nasze ThreadPool | Intel TBB, Tokio (Rust), Go runtime |

> Work stealing to temat na osobny wykład. Na razie wystarczy wiedzieć,
> że **istnieje** i **kiedy jest potrzebny** (≥16 rdzeni, dużo krótkich zadań).

---

## 10. Ćwiczenia praktyczne

### Ćwiczenie 1: Podstawowy Thread Pool 🔧

**Zadanie:** Zaimplementuj `ThreadPool` z sekcji 3.1 (wersja bez `future`)
od zera, **bez podglądania rozwiązania**.

**Wymagania:**
1. Konstruktor `ThreadPool(size_t num_threads)` — tworzy workerów
2. `submit(std::function<void()> task)` — dodaje zadanie
3. Destruktor — graceful shutdown (dokończ wszystkie zadania, potem join)
4. Thread-safe

**Test poprawności:**

```cpp
#include <atomic>
#include <cassert>
#include <chrono>

// Twoja implementacja:
// class ThreadPool { ... };

int main() {
    const int NUM_TASKS = 10'000;
    std::atomic<int> counter{0};

    {
        ThreadPool pool(4);

        for (int i = 0; i < NUM_TASKS; ++i) {
            pool.submit([&counter] {
                counter.fetch_add(1);
            });
        }

        // Destruktor ThreadPool — czeka na zakończenie wszystkich zadań
    }

    assert(counter == NUM_TASKS);
    std::cout << "✅ Wszystkie " << NUM_TASKS << " zadań wykonane!\n";

    // Test wielokrotnego tworzenia/niszczenia:
    for (int i = 0; i < 100; ++i) {
        ThreadPool pool(2);
        pool.submit([&counter] { counter++; });
    }
    assert(counter == NUM_TASKS + 100);
    std::cout << "✅ Wielokrotne tworzenie/niszczenie OK!\n";

    // Test z pustym poolem (brak submit):
    { ThreadPool pool(4); }
    std::cout << "✅ Pusty pool poprawnie zamknięty!\n";
}
```

---

### Ćwiczenie 2: Thread Pool z `std::future` 🎯

**Zadanie:** Rozszerz pool z ćwiczenia 1 o `submit()` zwracające `std::future`.

**Test:**

```cpp
int main() {
    ThreadPool pool(4);

    // Test 1: Zadanie zwracające int
    auto f1 = pool.submit([] { return 42; });
    assert(f1.get() == 42);

    // Test 2: Zadanie z argumentami
    auto f2 = pool.submit([](int a, int b) { return a * b; }, 6, 7);
    assert(f2.get() == 42);

    // Test 3: Zadanie void
    std::atomic<bool> done{false};
    auto f3 = pool.submit([&done] { done = true; });
    f3.get();
    assert(done);

    // Test 4: Wyjątek
    auto f4 = pool.submit([]() -> int {
        throw std::runtime_error("test error");
    });
    try {
        f4.get();
        assert(false && "Powinien rzucić wyjątek!");
    } catch (const std::runtime_error& e) {
        assert(std::string(e.what()) == "test error");
    }

    // Test 5: Wiele future naraz
    std::vector<std::future<int>> futures;
    for (int i = 0; i < 1000; ++i) {
        futures.push_back(pool.submit([i] { return i * i; }));
    }
    for (int i = 0; i < 1000; ++i) {
        assert(futures[i].get() == i * i);
    }

    std::cout << "✅ Wszystkie testy przeszły!\n";
}
```

---

### Ćwiczenie 3: Benchmark 📊

**Zadanie:** Zaimplementuj benchmark porównujący trzy podejścia:
1. `std::thread` per task
2. `std::async(std::launch::async, ...)`
3. Twój `ThreadPool`

**Tabela wyników do wypełnienia:**

| Liczba zadań | Thread per task | `std::async` | ThreadPool (Nt) | Speedup |
|:---:|:---:|:---:|:---:|:---:|
| 100 | ___ ms | ___ ms | ___ ms | ___x |
| 1 000 | ___ ms | ___ ms | ___ ms | ___x |
| 10 000 | ___ ms | ___ ms | ___ ms | ___x |
| 100 000 | ___ ms* | ___ ms | ___ ms | ___x |

*\* 100k wątków może nie dać się stworzyć — system odmówi. To właśnie dlatego
potrzebujemy puli!*

**Zmierz też dla różnych rozmiarów zadań:**

| Rozmiar zadania | Thread per task | ThreadPool | Speedup |
|:---:|:---:|:---:|:---:|
| 0 μs (puste) | ___ ms | ___ ms | ___x |
| 1 μs | ___ ms | ___ ms | ___x |
| 10 μs | ___ ms | ___ ms | ___x |
| 100 μs | ___ ms | ___ ms | ___x |
| 1 ms | ___ ms | ___ ms | ___x |

**Pytania:**
- Przy jakim rozmiarze zadania różnica między podejściami zaczyna zanikać?
- Dlaczego? (Podpowiedź: overhead tworzenia wątku ≈ 10-50 μs)
- Jaki jest optymalny rozmiar puli dla Twojego CPU?

---

### Ćwiczenie 4: Parallel Merge Sort 🔀

**Zadanie:** Zaimplementuj równoległy merge sort na Thread Pool.

```
Sekwencyjny merge sort:

  [8, 3, 5, 1, 9, 2, 7, 4]
         /           \
  [8, 3, 5, 1]   [9, 2, 7, 4]
    /      \        /      \
 [8, 3]  [5, 1]  [9, 2]  [7, 4]
  / \     / \     / \      / \
 [8] [3] [5] [1] [9] [2] [7] [4]
  \ /     \ /     \ /      \ /
 [3, 8]  [1, 5]  [2, 9]  [4, 7]
    \      /        \      /
  [1, 3, 5, 8]   [2, 4, 7, 9]
         \           /
  [1, 2, 3, 4, 5, 7, 8, 9]


Równoległy merge sort:

  Każdy podział → submit() na pool → future.get() na wynik
  Ale UWAGA na deadlock (sekcja 5.1)!
```

**Szkielet:**

```cpp
void parallel_merge_sort(ThreadPool& pool, std::vector<int>& data,
                         int depth = 0, int max_depth = 3)
{
    if (data.size() <= 1) return;

    size_t mid = data.size() / 2;
    std::vector<int> left(data.begin(), data.begin() + mid);
    std::vector<int> right(data.begin() + mid, data.end());

    if (depth < max_depth) {
        // Równolegle sortuj połówki
        auto future_left = pool.submit([&pool, &left, depth, max_depth] {
            parallel_merge_sort(pool, left, depth + 1, max_depth);
        });

        // Prawą połówkę sortuj w BIEŻĄCYM wątku
        //    (nie submit! → unikamy deadlocka)
        parallel_merge_sort(pool, right, depth + 1, max_depth);

        future_left.get();  // czekaj na lewą
    } else {
        // Za głęboko — sortuj sekwencyjnie
        std::sort(left.begin(), left.end());
        std::sort(right.begin(), right.end());
    }

    // Merge
    std::merge(left.begin(), left.end(),
               right.begin(), right.end(),
               data.begin());
}
```

**Pytania:**
1. Dlaczego `max_depth = 3` a nie `= 20`? (Hint: ile tasków = 2^depth)
2. Dlaczego prawą połówkę sortujemy w **bieżącym wątku** zamiast `submit()`?
3. Co się stanie gdy `pool` ma 2 wątki a `max_depth = 10`?
4. Zmierz speedup vs `std::sort()` — dla jakiego rozmiaru tablicy
   parallel merge sort wygrywa?

**Tabela wyników:**

| Rozmiar | `std::sort` | Parallel (2t) | Parallel (4t) | Parallel (8t) |
|:---:|:---:|:---:|:---:|:---:|
| 1 000 | ___ ms | ___ ms | ___ ms | ___ ms |
| 10 000 | ___ ms | ___ ms | ___ ms | ___ ms |
| 100 000 | ___ ms | ___ ms | ___ ms | ___ ms |
| 1 000 000 | ___ ms | ___ ms | ___ ms | ___ ms |
| 10 000 000 | ___ ms | ___ ms | ___ ms | ___ ms |

---

### Ćwiczenie 5: Web Crawler (symulacja) 🕷️

**Zadanie:** Zaimplementuj symulowany web crawler na Thread Pool.

**Scenariusz:**
- Każdy "URL" to `string`
- "Pobieranie" strony to `sleep(10-100ms)` + zwrócenie listy nowych URLi
- Crawler zaczyna od jednego URLa i odkrywa kolejne
- Max głębokość: 3 poziomy
- Nie odwiedzaj tego samego URLa dwa razy

```
URL: "site.com"
  ├─ "site.com/a"
  │   ├─ "site.com/a/1"
  │   └─ "site.com/a/2"
  ├─ "site.com/b"
  │   ├─ "site.com/b/1"
  │   └─ "site.com/a"      ← DUPLIKAT! Nie odwiedzaj ponownie!
  └─ "site.com/c"
      └─ "site.com/c/1"
```

**Szkielet:**

```cpp
#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <mutex>
#include <future>
#include <random>
#include <chrono>

// Symulacja pobierania strony
struct Page {
    std::string url;
    std::string content;
    std::vector<std::string> links;
};

Page fetch(const std::string& url) {
    // Symulacja opóźnienia sieciowego
    std::this_thread::sleep_for(std::chrono::milliseconds(
        10 + rand() % 90));

    Page page;
    page.url = url;
    page.content = "Content of " + url;

    // Generuj losowe linki
    if (url.length() < 30) {  // limit głęboko��ci
        for (int i = 0; i < 3; ++i) {
            page.links.push_back(url + "/" + std::to_string(i));
        }
    }
    return page;
}

class Crawler {
    ThreadPool& pool_;
    std::set<std::string> visited_;
    std::mutex visited_mutex_;
    std::atomic<int> pages_fetched_{0};

public:
    Crawler(ThreadPool& pool) : pool_(pool) {}

    void crawl(const std::string& start_url, int max_depth) {
        // TODO:
        // 1. Sprawdź czy URL był już odwiedzony (thread-safe!)
        // 2. Jeśli nie → submit(fetch) na pool
        // 3. Gdy fetch się zakończy → dla każdego linka: crawl(link, depth-1)
        // 4. Zbierz statystyki: ile stron, ile czasu
    }

    int pages_count() const { return pages_fetched_; }
};

int main() {
    ThreadPool pool(8);
    Crawler crawler(pool);

    auto start = std::chrono::steady_clock::now();
    crawler.crawl("site.com", 3);
    auto elapsed = std::chrono::steady_clock::now() - start;

    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
    std::cout << "Pobrano " << crawler.pages_count() << " stron w " << ms << " ms\n";

    // Porównaj z sekwencyjnym crawlerem:
    // Sekwencyjnie: N stron × ~50ms = N×50ms
    // Równolegle (8 wątków): ~N×50ms/8
}
```

**Pytania:**
1. Jak uniknąć race condition na `visited_`?
2. Czy ten crawler może wywołać deadlock w poolu? (Hint: `future.get()` w zadaniu!)
3. Jak rozwiązać problem z punktu 2? (Hint: nie czekaj na podzadania w zadaniu)

---

### Ćwiczenie 6: Thread Pool z monitoringiem 📈

**Zadanie:** Rozszerz ThreadPool o statystyki i monitoring.

```cpp
struct PoolStats {
    size_t total_tasks_submitted = 0;
    size_t total_tasks_completed = 0;
    size_t tasks_pending = 0;       // w kolejce
    size_t active_workers = 0;      // aktualnie wykonujących zadanie
    size_t idle_workers = 0;        // czekających na zadanie
    double avg_wait_time_us = 0;    // średni czas w kolejce
    double avg_exec_time_us = 0;    // średni czas wykonania
    double max_wait_time_us = 0;
};

class MonitoredThreadPool {
public:
    explicit MonitoredThreadPool(size_t num_threads);
    ~MonitoredThreadPool();

    void submit(std::function<void()> task);

    PoolStats stats() const;  // thread-safe snapshot statystyk

    // Wypisz statystyki na żywo (co 1s)
    void start_monitoring();
    void stop_monitoring();
};
```

**Oczekiwany output monitoringu:**

```
[00:01] pending=15 active=4/4 completed=85/100 avg_wait=230μs avg_exec=1.2ms
[00:02] pending=3  active=4/4 completed=197/200 avg_wait=180μs avg_exec=1.1ms
[00:03] pending=0  active=2/4 completed=200/200 avg_wait=150μs avg_exec=1.1ms ✅
```

---

### Ćwiczenie 7 (projekt): Równoległy procesor obrazów 🖼️

**Zadanie:** Zaimplementuj prosty procesor obrazów używający Thread Pool.

Obraz to macierz pikseli (uproszczona reprezentacja):

```cpp
struct Pixel {
    uint8_t r, g, b;
};

using Image = std::vector<std::vector<Pixel>>;
```

**Operacje do zaimplementowania:**

| Operacja | Opis | Typ |
|----------|------|-----|
| `brightness(img, delta)` | Zmień jasność o delta | Parallel map (per pixel) |
| `grayscale(img)` | Konwersja na odcienie szarości | Parallel map |
| `blur(img, radius)` | Rozmycie Gaussa | Parallel map (per wiersz) |
| `histogram(img)` | Histogram jasności (256 bins) | Parallel reduce |
| `average_color(img)` | Średni kolor obrazu | Parallel reduce |

**Wymagania:**
1. Każda operacja dzieli obraz na **chunki wierszy** i przetwarza je równolegle
2. Porównaj czas z wersją sekwencyjną dla obrazu 4000×4000

**Szkielet:**

```cpp
class ImageProcessor {
    ThreadPool& pool_;

public:
    ImageProcessor(ThreadPool& pool) : pool_(pool) {}

    // Zmień jasność — każdy wiersz niezależnie
    void brightness(Image& img, int delta) {
        std::vector<std::future<void>> futures;
        for (size_t row = 0; row < img.size(); ++row) {
            futures.push_back(pool_.submit([&img, row, delta] {
                for (auto& pixel : img[row]) {
                    pixel.r = std::clamp(pixel.r + delta, 0, 255);
                    pixel.g = std::clamp(pixel.g + delta, 0, 255);
                    pixel.b = std::clamp(pixel.b + delta, 0, 255);
                }
            }));
        }
        for (auto& f : futures) f.get();
    }

    // TODO: Zaimplementuj grayscale, blur, histogram, average_color
    // Hint: blur wymaga odczytu sąsiednich wierszy — uważaj na synchronizację!
};
```

**Tabela wyników:**

| Operacja | Sekwencyjnie | Pool (2t) | Pool (4t) | Pool (8t) | Speedup (8t) |
|:---:|:---:|:---:|:---:|:---:|:---:|
| brightness 4k×4k | ___ ms | ___ ms | ___ ms | ___ ms | ___x |
| grayscale 4k×4k | ___ ms | ___ ms | ___ ms | ___ ms | ___x |
| blur (r=5) 4k×4k | ___ ms | ___ ms | ___ ms | ___ ms | ___x |
| histogram 4k×4k | ___ ms | ___ ms | ___ ms | ___ ms | ___x |

**Pytania:**
- Dlaczego speedup nie jest dokładnie 8× na 8 wątkach?
- Która operacja skaluje się najlepiej? Dlaczego?
- Czy chunking po 1 wierszu jest optymalny? Spróbuj chunki po 10, 100, 500 wierszy.

---

### Ćwiczenie 8: Analiza pułapek 🕵️

**Zadanie teoretyczno-praktyczne.** Dla każdego poniższego kodu: określ czy jest poprawny.
Jeśli nie — jaki problem i jak naprawić?

**Kod A:**
```cpp
ThreadPool pool(4);
int result = 0;

for (int i = 0; i < 1000; ++i) {
    pool.submit([&result, i] {
        result += i;  // ???
    });
}
```

<details>
<summary>Odpowiedź A</summary>

❌ **Data race na `result`!** Wiele wątków pisze do `result` bez synchronizacji.

Naprawa: `std::atomic<int> result{0};` i `result.fetch_add(i);`

Albo: użyj `submit()` z `future`, zbierz wyniki, zsumuj w jednym wątku.

</details>

**Kod B:**
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

<details>
<summary>Odpowiedź B</summary>

❌ **Deadlock!** Pool ma 2 wątki. f1 i f3 zajmują oba wątki.
Każdy czeka na podzadanie (f2, f4) — ale nie ma wolnych wątków!

Naprawa: zwiększ pool do ≥4 wątków, lub nie rób `get()` wewnątrz zadania.

</details>

**Kod C:**
```cpp
ThreadPool pool(4);
std::vector<int> data = {1, 2, 3, 4, 5};

auto f = pool.submit([&data] {
    return std::accumulate(data.begin(), data.end(), 0);
});

data.clear();  // ???

std::cout << f.get() << "\n";
```

<details>
<summary>Odpowiedź C</summary>

❌ **Data race / dangling reference!** `data.clear()` może wykonać się
ZANIM task zacznie czytać `data`. Task ma referencję do wyczyszczonego wektora.

Naprawa: przechwytuj `data` przez **kopię** w lambdzie: `[data]` zamiast `[&data]`.
Albo: zrób `f.get()` PRZED `data.clear()`.

</details>

**Kod D:**
```cpp
{
    ThreadPool pool(4);
    for (int i = 0; i < 100; ++i) {
        pool.submit([i] {
            std::this_thread::sleep_for(std::chrono::seconds(10));
            std::cout << i << "\n";
        });
    }
    // pool wychodzi ze scope...
}
std::cout << "Koniec\n";
```

<details>
<summary>Odpowiedź D</summary>

✅ **Poprawne, ale wolne!** Destruktor `~ThreadPool()` czeka aż WSZYSTKIE
zadania się zakończą (graceful shutdown). Program będzie czekał
~10s × 100/4 = ~250 sekund!

To nie bug, to feature — ale trzeba być świadomym, że destruktor blokuje.
Jeśli nie chcesz czekać → dodaj metodę `cancel_pending()` która wyrzuca
nieuruchomione taski z kolejki.

</details>

---

## 11. Powiązania ze standardem C++ i przyszłość

| Standard | Narzędzie | Znaczenie |
|----------|-----------|-----------|
| C++11 | `std::thread` | Tworzenie wątków |
| C++11 | `std::future`, `std::promise` | Asynchroniczne wyniki |
| C++11 | `std::packaged_task` | Opakowanie callable → future |
| C++11 | `std::async` | Prosty (ale ograniczony) async |
| C++11 | `std::function` | Type-erased callable (kolejka zadań) |
| C++17 | `std::invoke_result_t` | Dedukowanie typu zwracanego |
| C++17 | Parallel STL (`std::execution::par`) | `std::sort(std::execution::par, ...)` — wewnętrznie thread pool! |
| C++20 | `std::jthread` + `std::stop_token` | Kooperatywne anulowanie |
| C++20 | Coroutines (`co_await`) | Task bez blokowania wątku |
| C++23 | `std::execution` (P2300) | **Standaryzowany thread pool!** (Senders/Receivers) |

### C++23 `std::execution` — przyszłość

```cpp
// C++23 (propozycja P2300):
#include <execution>

auto pool = std::execution::static_thread_pool(4);
auto scheduler = pool.get_scheduler();

auto task = std::execution::schedule(scheduler)
          | std::execution::then([] { return 42; })
          | std::execution::then([](int x) { return x * 2; });

auto result = std::this_thread::sync_wait(task);
// result = 84
```

> Na razie P2300 nie jest jeszcze w pełni zaimplementowane w kompilatorach,
> ale to **przyszłość** — ustandaryzowany model asynchroniczności w C++.

---

## 12. Podsumowanie

### Kluczowe wnioski

1. **Thread Pool = producent-konsument** — kolejka zadań + stała pula workerów
2. **`std::packaged_task` + `std::future`** pozwalają zwracać wyniki z zadań
3. **Tworzenie wątku jest drogie** — pool amortyzuje ten koszt do zera
4. **Backpressure** (ograniczona kolejka) chroni przed wyczerpaniem pamięci
5. **Deadlock jest możliwy** jeśli zadanie czeka na wynik podzadania na tym samym poolu
6. **Przechwytuj przez kopię** w lambdach — unikaj dangling references
7. **Kolejność destrukcji** ma znaczenie — pool powinien być niszczony pierwszy

### Reguła kciuka

```
Potrzebujesz równoległości?
│
├─ Jedno zadanie fire-and-forget?
│  └─ std::async(std::launch::async, ...) ✅
│
├─ Kilka niezależnych zadań?
│  └─ std::async lub prosty ThreadPool ✅
│
├─ Wiele zadań (>100), potrzebna kontrola?
│  ├─ CPU-bound?
│  │  └─ ThreadPool(hardware_concurrency()) ✅
│  └─ I/O-bound?
│     └─ ThreadPool(2-10 × hardware_concurrency()) ✅
│
├─ Potrzebujesz backpressure?
│  └─ BoundedThreadPool z ograniczoną kolejką ✅
│
├─ Zadania mają priorytety?
│  └─ PriorityThreadPool ✅
│
└─ 64+ rdzeni, mikrosekundowe zadania?
   └─ Work-stealing pool (Intel TBB / custom) ✅
```

### Relacja do poprzednich tematów

```
┌─────────────────────────────────────────────────────────┐
│ Zajęcia 1: Producent-Konsument                          │
│   BoundedBuffer, mutex, condition_variable, shutdown    │
│         │                                               │
│         ▼                                               │
│ Zajęcia 4: Thread Pool  (= producent-konsument!)        │
│   kolejka zadań = BoundedBuffer<function<void()>>       │
│   submit() = push(), worker_loop() = pop()              │
│   graceful shutdown = close()                           │
├─────────────────────────────────────────────────────────┤
│ Zajęcia 2: Ucztujący Filozofowie                        │
│   → Pułapka deadlocka w poolu (task czeka na subtask)   │
│   → scoped_lock w parallel operacjach na wielu zasobach │
├─────────────────────────────────────────────────────────┤
│ Zajęcia 3: Czytelnicy-Pisarze                           │
│   → shared_mutex w Crawlerze (visited set: read/write)  │
│   → RCU w cache aktualizowanym przez pool               │
└─────────────────────────────────────────────────────────┘
```

---

## Literatura i zasoby

- A. Williams, *C++ Concurrency in Action*, 2nd ed. (Manning, 2019) — rozdział 9: Thread Pools
- C++ Reference: [std::async](https://en.cppreference.com/w/cpp/thread/async),
  [std::packaged_task](https://en.cppreference.com/w/cpp/thread/packaged_task),
  [std::future](https://en.cppreference.com/w/cpp/thread/future)
- CppCon 2015: Sean Parent, *Better Code: Concurrency*
- CppCon 2019: Anthony Williams, *An Introduction to Multithreading in C++20*
- P2300: `std::execution` — [wg21.link/P2300](https://wg21.link/P2300)
- Intel TBB documentation (work-stealing scheduler)
