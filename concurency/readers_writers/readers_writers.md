# Czytelnicy-Pisarze (Readers-Writers)
## Notatki do wykładu/ćwiczeń z programowania współbieżnego w C++

---

## 1. Wprowadzenie — o czym jest ten problem?

**Problem czytelników i pisarzy** został sformułowany przez Courtois, Heymans i Parnas (1971).
Opisuje sytuację, w której wiele wątków współdzieli zasób (np. bazę danych, plik konfiguracyjny,
pamięć podręczną), przy czym:

- **Czytelnicy** — tylko odczytują dane, **nie modyfikują** ich
- **Pisarze** — modyfikują dane

### Kluczowa obserwacja

| Operacja | Czytelnik | Pisarz |
|----------|:---------:|:------:|
| **Czytelnik** | ✅ Mogą działać **jednocześnie** | ❌ Muszą się wykluczać |
| **Pisarz** | ❌ Muszą się wykluczać | ❌ Muszą się wykluczać |

> **Wielu czytelników naraz** — bo odczyt nie zmienia danych.
> **Pisarz musi być sam** — bo zapis + odczyt jednocześnie = data race.

### Przykłady z życia

| Czytelnicy | Pisarze | Współdzielony zasób |
|------------|---------|---------------------|
| Wątki obsługujące HTTP GET | Wątek aktualizujący konfigurację | `std::map<string, string> config` |
| Wątki renderujące scenę gry | Wątek physics engine | Pozycje obiektów |
| Wątki odpytujące cache | Wątek odświeżający cache | `std::unordered_map` |
| `SELECT` w bazie danych | `UPDATE`/`INSERT` | Tabela |
| Wiele wątków DNS lookup | Wątek odświeżający wpisy DNS | Tablica DNS |

### Dlaczego nie po prostu `std::mutex`?

```cpp
std::mutex mtx;
std::string shared_data = "Hello";

void czytelnik(int id) {
    std::lock_guard<std::mutex> lock(mtx);
    // Tylko ODCZYT — ale i tak blokujemy WSZYSTKICH
    std::cout << "Czytelnik " << id << ": " << shared_data << "\n";
}
```

```
Czytelnik A: lock ████████ unlock
Czytelnik B:                      lock ████████ unlock     ← CZEKAŁ niepotrzebnie!
Czytelnik C:                                                lock ████████ unlock
                ──────────────────────────────────────────────────────▶ czas

Z std::mutex: czytelnik blokuje czytelnika ❌ (niepotrzebne!)

Z rozwiązaniem readers-writers:

Czytelnik A: ████████
Czytelnik B: ████████          ← RÓWNOLEGLE! ✅
Czytelnik C: ████████
                ──────────────▶ czas     3× szybciej!
```

---

## 2. Warianty problemu

Są **trzy klasyczne warianty**, różniące się tym, kogo faworyzujemy:

### Wariant 1: Preferencja czytelników (readers-preference)
- Dopóki jest choć jeden czytelnik, kolejni czytelnicy **wchodzą natychmiast**
- Pisarz czeka aż **wszyscy** czytelnicy skończą
- ⚠️ **Zagłodzenie pisarzy** — ciągły napływ czytelników = pisarz nigdy nie wejdzie

### Wariant 2: Preferencja pisarzy (writers-preference)
- Gdy pisarz czeka, **nowi czytelnicy są blokowani**
- Pisarz wchodzi gdy aktywni czytelnicy skończą
- ⚠️ **Zagłodzenie czytelników** — ciągły napływ pisarzy = czytelnicy nigdy nie wejdą

### Wariant 3: Fair (bez zagłodzenia)
- Obsługa w kolejności zgłoszeń (FIFO)
- Żaden wątek nie jest faworyzowany
- ⚠️ Najtrudniejszy do zaimplementowania

---

## 3. Implementacja ręczna — readers-preference

Zaczniemy od ręcznej implementacji z `std::mutex` + `std::condition_variable`,
żeby **zrozumieć mechanizm** zanim użyjemy `std::shared_mutex`.

### 3.1 Wersja naiwna (❌ BŁĘDNA — data race)

```cpp
// ❌ ŹLE — nie rób tego!
int reader_count = 0;  // ilu czytelników jest aktywnych?

void czytelnik() {
    reader_count++;            // ❌ data race!
    // ... czytanie ...
    reader_count--;            // ❌ data race!
}

void pisarz() {
    while (reader_count > 0);  // ❌ busy wait + data race!
    // ... pisanie ...
}
```

> **Ćwiczenie** Ile różnych rodzajów błędów widzisz w tym kodzie?
> (Odpowiedź: data race na `reader_count`, busy waiting, brak wzajemnego
> wykluczania pisarzy)

### 3.2 Poprawna implementacja ręczna

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <chrono>

class ReadersWritersLock {
    std::mutex mtx_;
    std::condition_variable cv_;

    int active_readers_ = 0;    // ilu czytelników aktualnie czyta
    bool writer_active_ = false; // czy pisarz jest aktywny

public:
    // ─── Interfejs czytelnika ───

    void read_lock() {
        std::unique_lock<std::mutex> lock(mtx_);
        // Czekaj aż żaden pisarz nie jest aktywny
        cv_.wait(lock, [this] {
            return !writer_active_;
        });
        ++active_readers_;
    }

    void read_unlock() {
        std::unique_lock<std::mutex> lock(mtx_);
        --active_readers_;
        if (active_readers_ == 0) {
            // Ostatni czytelnik — obudź czekającego pisarza
            cv_.notify_all();
        }
    }

    // ─── Interfejs pisarza ───

    void write_lock() {
        std::unique_lock<std::mutex> lock(mtx_);
        // Czekaj aż:
        //  - żaden czytelnik nie czyta
        //  - żaden inny pisarz nie pisze
        cv_.wait(lock, [this] {
            return active_readers_ == 0 && !writer_active_;
        });
        writer_active_ = true;
    }

    void write_unlock() {
        std::unique_lock<std::mutex> lock(mtx_);
        writer_active_ = false;
        // Obudź WSZYSTKICH — zarówno czytelników jak i pisarzy
        cv_.notify_all();
    }
};

// ─── Użycie ───

ReadersWritersLock rw;
std::string shared_data = "Wersja 0";

void czytelnik(int id) {
    for (int i = 0; i < 5; ++i) {
        rw.read_lock();
        std::cout << "Czytelnik " << id << " czyta: " << shared_data << "\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        rw.read_unlock();

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void pisarz(int id) {
    for (int i = 0; i < 3; ++i) {
        rw.write_lock();
        shared_data = "Wersja " + std::to_string(id * 100 + i);
        std::cout << "  ✏️ Pisarz " << id << " zapisał: " << shared_data << "\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        rw.write_unlock();

        std::this_thread::sleep_for(std::chrono::milliseconds(30));
    }
}

int main() {
    std::vector<std::thread> threads;
    for (int i = 1; i <= 5; ++i) threads.emplace_back(czytelnik, i);
    for (int i = 1; i <= 2; ++i) threads.emplace_back(pisarz, i);
    for (auto& t : threads) t.join();
}
```

### 3.3 Analiza: dlaczego to jest readers-preference?

```
Stan: writer_active_ = false, active_readers_ = 3
      Pisarz P czeka w write_lock() (active_readers_ != 0)

Nowy czytelnik R przychodzi:
  read_lock(): !writer_active_ → !false → true ✅
  → R WCHODZI natychmiast, active_readers_ = 4
  → P dalej czeka!

Kolejny czytelnik R2 przychodzi:
  → To samo — wchodzi natychmiast, active_readers_ = 5
  → P DALEJ CZEKA!

Jeśli czytelnicy przychodzą ciągle → P czeka W NIESKOŃCZONOŚĆ ☠️
```

> **⚠️ ZAGŁODZENIE PISARZA:** Ciągły strumień czytelników oznacza, że
> `active_readers_` nigdy nie spadnie do 0.

---

## 4. `std::shared_mutex` — rozwiązanie z biblioteki standardowej (C++17)

C++17 daje nam gotowe narzędzie:

```cpp
#include <shared_mutex>

std::shared_mutex rw_mutex;
std::string shared_data = "Wersja 0";

void czytelnik(int id) {
    // shared_lock — wielu czytelników jednocześnie
    std::shared_lock<std::shared_mutex> lock(rw_mutex);
    std::cout << "Czytelnik " << id << ": " << shared_data << "\n";
}

void pisarz(int id) {
    // unique_lock — wyłączny dostęp
    std::unique_lock<std::shared_mutex> lock(rw_mutex);
    shared_data = "Wersja " + std::to_string(id);
}
```

### Zestawienie typów locków

| Typ locka | Mutex | Współdzielenie | Użycie |
|-----------|-------|:--------------:|--------|
| `std::lock_guard<std::mutex>` | exclusive | ❌ | Prosty exclusive |
| `std::unique_lock<std::mutex>` | exclusive | ❌ | Exclusive + CV |
| `std::unique_lock<std::shared_mutex>` | **exclusive** | ❌ | **Pisarz** |
| `std::shared_lock<std::shared_mutex>` | **shared** | ✅ | **Czytelnik** |
| `std::scoped_lock<std::shared_mutex>` | **exclusive** | ❌ | Pisarz (RAII) |

### Zachowanie `std::shared_mutex` — tabela stanów

```
                        Żądanie shared_lock    Żądanie unique_lock
                        (nowy czytelnik)       (nowy pisarz)
Stan mutexu             ───────────────────    ────────────────────
Wolny                   ✅ Wchodzi             ✅ Wchodzi
Shared (N czytelników)  ✅ Wchodzi (N+1)       ❌ Czeka
Exclusive (pisarz)      ❌ Czeka               ❌ Czeka
```

### Ważne: jaki wariant implementuje `std::shared_mutex`?

**Standard C++ tego NIE SPECYFIKUJE!** Zależy od implementacji:

| Implementacja | Wariant | Uwagi |
|--------------|---------|-------|
| libstdc++ (GCC, Linux) | Zwykle writers-preference (pthread_rwlock) | Zależy od konfiguracji pthreads |
| libc++ (Clang) | Zależy od platformy | |
| MSVC (Windows) | SRW Lock — ani readers ani writers preference | Bliżej fair, ale bez gwarancji |

> **Wniosek:** Nie możesz polegać na konkretnym zachowaniu `std::shared_mutex`
> w kwestii fairness — jeśli potrzebujesz gwarancji, musisz implementować sam.

---

## 5. Zagłodzenie — analiza i rozwiązania

### 5.1 Demonstracja zagłodzenia pisarza

```cpp
#include <iostream>
#include <thread>
#include <shared_mutex>
#include <vector>
#include <atomic>
#include <chrono>

std::shared_mutex rw_mutex;
int shared_data = 0;
std::atomic<int> reader_entries{0};
std::atomic<int> writer_entries{0};

void aggressive_reader(int id) {
    // Czytelnik który CIĄGLE czyta — nie daje pisarzowi szansy
    for (int i = 0; i < 10'000; ++i) {
        std::shared_lock lock(rw_mutex);
        volatile int tmp = shared_data;  // odczyt
        (void)tmp;
        reader_entries++;
    }
}

void starving_writer(int id) {
    for (int i = 0; i < 10; ++i) {
        auto start = std::chrono::steady_clock::now();

        std::unique_lock lock(rw_mutex);  // ← jak długo tu czekamy?

        auto wait_time = std::chrono::steady_clock::now() - start;
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(wait_time).count();

        shared_data++;
        writer_entries++;
        std::cout << "Pisarz " << id << " wszedł po " << ms << " ms czekania\n";
    }
}

int main() {
    std::vector<std::thread> threads;

    // 10 agresywnych czytelników vs 1 pisarz
    for (int i = 0; i < 10; ++i) threads.emplace_back(aggressive_reader, i);
    threads.emplace_back(starving_writer, 0);

    for (auto& t : threads) t.join();

    std::cout << "\nOdczyty: " << reader_entries
              << "\nZapisy:  " << writer_entries << "\n";
}
```

### Zadanie dla studentów (przy komputerze):
> Skompiluj i uruchom powyższy program kilka razy. Zaobserwuj czasy czekania pisarza.
> Czy są stabilne? Czy pisarz w ogóle się doczekał?

### 5.2 Implementacja writers-preference

```cpp
class WritersPreferenceRWLock {
    std::mutex mtx_;
    std::condition_variable cv_readers_;
    std::condition_variable cv_writers_;

    int active_readers_ = 0;
    int waiting_writers_ = 0;   // ← KLUCZ: liczymy czekających pisarzy
    bool writer_active_ = false;

public:
    void read_lock() {
        std::unique_lock<std::mutex> lock(mtx_);
        // Czytelnik czeka jeśli:
        //  - pisarz jest aktywny, LUB
        //  - pisarz CZEKA (dajemy mu priorytet!)
        cv_readers_.wait(lock, [this] {
            return !writer_active_ && waiting_writers_ == 0;
            //                        ^^^^^^^^^^^^^^^^^^^^^^^^
            //                        TO jest kluczowa różnica!
        });
        ++active_readers_;
    }

    void read_unlock() {
        std::unique_lock<std::mutex> lock(mtx_);
        --active_readers_;
        if (active_readers_ == 0) {
            cv_writers_.notify_one();  // obudź jednego pisarza
        }
    }

    void write_lock() {
        std::unique_lock<std::mutex> lock(mtx_);
        ++waiting_writers_;    // zasygnalizuj: "pisarz czeka"
        cv_writers_.wait(lock, [this] {
            return active_readers_ == 0 && !writer_active_;
        });
        --waiting_writers_;
        writer_active_ = true;
    }

    void write_unlock() {
        std::unique_lock<std::mutex> lock(mtx_);
        writer_active_ = false;

        if (waiting_writers_ > 0) {
            cv_writers_.notify_one();   // priorytet: następny pisarz
        } else {
            cv_readers_.notify_all();   // brak pisarzy → wpuść czytelników
        }
    }
};
```

### 5.3 Wizualizacja — porównanie wariantów

```
Scenariusz: R1 czyta, P1 czeka, R2 przychodzi, R3 przychodzi

═══ READERS-PREFERENCE ═══

R1: ██████████████████████████
R2:      ███████████████████████████      ← wchodzi natychmiast
R3:              █████████████████████████  ← wchodzi natychmiast
P1:                                        ██████  ← czekał na WSZYSTKICH
                ─────────────────────────────────▶ czas


═══ WRITERS-PREFERENCE ═══

R1: █████████████
R2:               ░░░░░░░░░░░░░░░░░░░░░░░░███████  ← czekał!
R3:               ░░░░░░░░░░░░░░░░░░░░░░░░░███████  ← czekał!
P1:               ██████████                          ← wszedł gdy R1 skończył
                ─────────────────────────────────▶ czas
    (░ = czekanie, █ = praca)
    R2/R3 zablokowane bo waiting_writers_ > 0


═══ FAIR (FIFO) ═══

R1: █████████████
P1:               ██████████              ← wszedł jako pierwszy w kolejce
R2:                          ████████     ← swojak kolej
R3:                          ████████     ← równolegle z R2 (obaj czytelnicy)
                ─────────────────────────────────▶ czas
```

### 5.4 Implementacja fair (FIFO)

```cpp
#include <mutex>
#include <condition_variable>
#include <queue>
#include <thread>

class FairRWLock {
    enum class RequestType { READ, WRITE };

    struct Request {
        RequestType type;
        std::condition_variable cv;
        bool granted = false;
    };

    std::mutex mtx_;
    std::queue<Request*> wait_queue_;

    int active_readers_ = 0;
    bool writer_active_ = false;

    // Próbuje przyznać dostęp następnym w kolejce
    void try_grant_next(std::unique_lock<std::mutex>& /*lock*/) {
        while (!wait_queue_.empty()) {
            Request* front = wait_queue_.front();

            if (front->type == RequestType::WRITE) {
                // Pisarz może wejść tylko gdy nikt nie jest aktywny
                if (active_readers_ == 0 && !writer_active_) {
                    wait_queue_.pop();
                    writer_active_ = true;
                    front->granted = true;
                    front->cv.notify_one();
                }
                // Pisarz na froncie kolejki BLOKUJE dalszych czytelników
                return;

            } else {  // READ
                if (!writer_active_) {
                    wait_queue_.pop();
                    ++active_readers_;
                    front->granted = true;
                    front->cv.notify_one();
                    // Kontynuuj — może następny też jest czytelnikiem
                } else {
                    return;  // pisarz aktywny — czekaj
                }
            }
        }
    }

public:
    void read_lock() {
        std::unique_lock<std::mutex> lock(mtx_);

        // Szybka ścieżka: nikt nie czeka i pisarz nieaktywny
        if (wait_queue_.empty() && !writer_active_) {
            ++active_readers_;
            return;
        }

        // Wolna ścieżka: stań w kolejce
        Request req{RequestType::READ};
        wait_queue_.push(&req);
        req.cv.wait(lock, [&req] { return req.granted; });
    }

    void read_unlock() {
        std::unique_lock<std::mutex> lock(mtx_);
        --active_readers_;
        try_grant_next(lock);
    }

    void write_lock() {
        std::unique_lock<std::mutex> lock(mtx_);

        // Szybka ścieżka
        if (wait_queue_.empty() && active_readers_ == 0 && !writer_active_) {
            writer_active_ = true;
            return;
        }

        // Wolna ścieżka
        Request req{RequestType::WRITE};
        wait_queue_.push(&req);
        req.cv.wait(lock, [&req] { return req.granted; });
    }

    void write_unlock() {
        std::unique_lock<std::mutex> lock(mtx_);
        writer_active_ = false;
        try_grant_next(lock);
    }
};
```

### Kluczowa logika fair: grupowanie czytelników

```
Kolejka: [R, R, R, W, R, R]
          ─────── │  ─────
          grupa 1 │  grupa 2 (zablokowana przez W)

try_grant_next():
  → R granted ✅ (active_readers_ = 1)
  → R granted ✅ (active_readers_ = 2)
  → R granted ✅ (active_readers_ = 3)
  → W na froncie, ale active_readers_ > 0 → STOP
  
Gdy wszyscy trzej R skończą:
  → W granted ✅ (writer_active_ = true)
  
Gdy W skończy:
  → R granted ✅
  → R granted ✅
  → Kolejka pusta
```

---

## 6. Zagrożenia i pułapki

### 6.1 Upgrade locka: shared → exclusive (❌ DEADLOCK!)

```cpp
std::shared_mutex rw_mutex;

void read_then_write() {
    std::shared_lock<std::shared_mutex> read_lock(rw_mutex);
    int value = shared_data;  // czytanie OK

    if (value < 0) {
        // ❌ CHCĘ ZAPISAĆ — ale mam shared_lock!
        // Nie mogę "upgrade'ować" shared_lock do unique_lock w C++!
        
        // ❌ DEADLOCK:
        std::unique_lock<std::shared_mutex> write_lock(rw_mutex);
        // ^ czeka na exclusive, ale JA SAM trzymam shared!
    }
}
```

### Rozwiązanie: zwolnij i zablokuj ponownie

```cpp
void read_then_write_correct() {
    int value;
    {
        std::shared_lock<std::shared_mutex> read_lock(rw_mutex);
        value = shared_data;
    }   // ← shared_lock zwolniony

    if (value < 0) {
        std::unique_lock<std::shared_mutex> write_lock(rw_mutex);
        // ⚠️ UWAGA: shared_data mogło się zmienić od czasu odczytu!
        // Musisz sprawdzić warunek ponownie:
        if (shared_data < 0) {
            shared_data = 0;
        }
    }
}
```

> **Wzorzec:** Odczytaj → Zwolnij → Zablokuj exclusive → **Sprawdź ponownie** → Zapisz
> (analogia: double-checked locking)

### 6.2 Rekurencyjny read_lock

```cpp
void function_a() {
    std::shared_lock lock(rw_mutex);
    // ... czytanie ...
    function_b();  // ← wywołuje inną funkcję
}

void function_b() {
    std::shared_lock lock(rw_mutex);  // ← DRUGI shared_lock na tym samym mutexie
    // Czy to OK?
}
```

**Odpowiedź:** `std::shared_mutex` **pozwala** na wiele równoczesnych `shared_lock`,
więc to **zadziała** jeśli oba locki są z **różnych wątków**. Ale jeśli to **ten sam wątek**
(rekursja) — **standard mówi: undefined behavior!**

> `std::shared_mutex` **nie jest rekurencyjny** — ani w trybie shared, ani exclusive.
> Nie ma `std::recursive_shared_mutex` w standardzie.

### 6.3 Wydajność: shared_mutex nie zawsze jest szybszy

```
Benchmark: 8 wątków, 100% odczyty, krótka sekcja krytyczna

std::mutex:           450 ns/operację
std::shared_mutex:    380 ns/operację    ← tylko ~15% szybciej!

Dlaczego? shared_mutex ma wewnętrznie WIĘCEJ logiki:
  - Atomowy licznik czytelników
  - Sprawdzanie flagi pisarza
  - Potencjalnie: cache line bouncing przy aktualizacji licznika
```

**Reguła kciuka:**

| Scenariusz | Lepszy wybór |
|------------|-------------|
| 90%+ odczytów, sekcja krytyczna > 1μs | `std::shared_mutex` ✅ |
| 50/50 odczyty/zapisy | `std::mutex` ✅ (prostszy, mniejszy overhead) |
| Bardzo krótka sekcja krytyczna (< 100ns) | `std::mutex` ✅ |
| Jeden wątek pisarza, wiele czytelników | `std::shared_mutex` ✅ |

### 6.4 Inwersja priorytetów

```
Wątek niskiego priorytetu: trzyma shared_lock (czyta)
Wątek wysokiego priorytetu: czeka na unique_lock (chce pisać)

→ Wątek wysokiego priorytetu jest ZABLOKOWANY przez wątek niskiego priorytetu!
→ Klasyczna inwersja priorytetów
→ OS może nie wiedzieć o tej zależności (mutex w user-space)
```

---

## 7. Wzorce użycia `std::shared_mutex` w praktyce

### 7.1 Thread-safe cache

```cpp
#include <shared_mutex>
#include <unordered_map>
#include <string>
#include <optional>

class ThreadSafeCache {
    mutable std::shared_mutex mutex_;
    std::unordered_map<std::string, std::string> cache_;

public:
    // Odczyt — shared_lock (wielu jednocześnie)
    std::optional<std::string> get(const std::string& key) const {
        std::shared_lock lock(mutex_);
        auto it = cache_.find(key);
        if (it != cache_.end()) return it->second;
        return std::nullopt;
    }

    // Zapis — unique_lock (wyłączny)
    void put(const std::string& key, const std::string& value) {
        std::unique_lock lock(mutex_);
        cache_[key] = value;
    }

    // Usuwanie — unique_lock
    bool remove(const std::string& key) {
        std::unique_lock lock(mutex_);
        return cache_.erase(key) > 0;
    }

    // Rozmiar — shared_lock (tylko odczyt)
    size_t size() const {
        std::shared_lock lock(mutex_);
        return cache_.size();
    }
};
```

> **Zwróć uwagę na `mutable`** — `mutex_` musi być `mutable`, bo `get()` i `size()`
> są `const`, ale muszą blokować mutex.

### 7.2 Read-copy-update (RCU-like pattern)

Wzorzec dla ekstremalnie dużej liczby odczytów i rzadkich zapisów:

```cpp
#include <shared_mutex>
#include <memory>
#include <atomic>

class RCUConfig {
    // shared_ptr jest atomowy od C++20 (std::atomic<std::shared_ptr<T>>)
    // Dla C++17 używamy shared_mutex:
    mutable std::shared_mutex mutex_;
    std::shared_ptr<const std::map<std::string, std::string>> config_;

public:
    RCUConfig()
        : config_(std::make_shared<const std::map<std::string, std::string>>()) {}

    // Odczyt — kopiujemy shared_ptr (szybkie, atomowe)
    std::shared_ptr<const std::map<std::string, std::string>> read() const {
        std::shared_lock lock(mutex_);
        return config_;  // kopia shared_ptr — czytelnicy mogą trzymać długo
    }

    // Zapis — tworzymy NOWĄ kopię, podmieniamy wskaźnik
    void update(const std::string& key, const std::string& value) {
        std::unique_lock lock(mutex_);
        // Kopia istniejącej mapy
        auto new_config = std::make_shared<std::map<std::string, std::string>>(*config_);
        (*new_config)[key] = value;
        config_ = new_config;  // podmiana wskaźnika (atomowa)
        // Starzy czytelnicy dalej trzymają starą wersję — to OK!
    }
};
```

Natomiast w c++20 można
```cpp
#include <atomic>
#include <memory>
#include <map>
#include <string>

class RCUConfig {
    std::atomic<std::shared_ptr<const std::map<std::string, std::string>>> config_;

public:
    RCUConfig()
        : config_(std::make_shared<const std::map<std::string, std::string>>()) {}

    // Odczyt — atomowa kopia shared_ptr, BEZ LOCKA
    std::shared_ptr<const std::map<std::string, std::string>> read() const {
        return config_.load();  // atomowy odczyt
    }

    // Zapis — kopiuj, modyfikuj, podmień
    void update(const std::string& key, const std::string& value) {
        auto old_config = config_.load();
        std::shared_ptr<std::map<std::string, std::string>> new_config;

        do {
            new_config = std::make_shared<std::map<std::string, std::string>>(*old_config);
            (*new_config)[key] = value;
            // Próbuj podmienić — jeśli ktoś inny zdążył zmienić, powtórz
        } while (!config_.compare_exchange_weak(old_config, new_config));
    }
};
```



---

## 8. Porównanie podejść — tabela podsumowująca

| Podejście | Fairness | Zagłodzenie | Przepustowość odczytów | Złożoność |
|-----------|:--------:|:-----------:|:----------------------:|:---------:|
| `std::mutex` (baseline) | N/A | N/A | ❌ Sekwencyjna | ⭐ |
| Ręczne readers-pref. | Czytelnicy ✅ | Pisarze ☠️ | ✅✅✅ | ⭐⭐⭐ |
| Ręczne writers-pref. | Pisarze ✅ | Czytelnicy ☠️ | ✅✅ | ⭐⭐⭐ |
| Ręczne fair (FIFO) | ✅ Obie strony | ✅ Brak | ✅✅ | ⭐⭐⭐⭐⭐ |
| `std::shared_mutex` | Zależy od impl. | Możliwe | ✅✅ | ⭐⭐ |
| RCU-like pattern | ✅ | ✅ Brak | ✅✅✅✅ | ⭐⭐⭐ |

---

## 9. Ćwiczenia praktyczne

### Ćwiczenie 1: Znajdź błędy 🔍

W poniższym kodzie jest **5 błędów**. Znajdź je wszystkie.

```cpp
#include <shared_mutex>
#include <thread>
#include <vector>
#include <iostream>

std::shared_mutex rw;
std::vector<int> data = {1, 2, 3, 4, 5};

void reader(int id) {
    std::unique_lock<std::shared_mutex> lock(rw);  // 🐛 #1?
    int sum = 0;
    for (int x : data) sum += x;
    std::cout << "Reader " << id << ": sum=" << sum << "\n";
}

void writer(int id) {
    std::shared_lock<std::shared_mutex> lock(rw);  // 🐛 #2?
    data.push_back(id);
    std::cout << "Writer " << id << ": added " << id << "\n";
}

void read_and_modify(int id) {
    std::shared_lock<std::shared_mutex> slock(rw);
    if (data.size() > 10) {
        std::unique_lock<std::shared_mutex> ulock(rw);  // 🐛 #3?
        data.clear();
    }
}

int main() {
    std::thread t1(reader, 1);
    std::thread t2(writer, 1);
    std::thread t3(read_and_modify, 1);
    // 🐛 #4? #5?
}
```

<details>
<summary>Odpowiedzi</summary>

1. **Reader używa `unique_lock`** — powinien `shared_lock` (blokuje innych czytelników niepotrzebnie)
2. **Writer używa `shared_lock`** — powinien `unique_lock` (data race! wielu "pisarzy" jednocześnie)
3. **Deadlock w `read_and_modify`** — trzyma `shared_lock` i próbuje wziąć `unique_lock` na tym samym mutexie
4. **Brak `join()`** — program kończy się z aktywnymi wątkami → `std::terminate()`
5. **Brak `join()` na `t3`** — to samo, dla każdego wątku

</details>

---

### Ćwiczenie 2: Zmierz zagłodzenie 📊

Zaimplementuj program testowy który:

1. Tworzy **N czytelników** i **M pisarzy** (parametry z linii komend)
2. Każdy czytelnik wykonuje 10000 odczytów
3. Każdy pisarz wykonuje 100 zapisów
4. Mierzy dla każdego wątku:
   - Łączny czas oczekiwania na lock
   - Liczbę udanych operacji
   - Maksymalny czas pojedynczego oczekiwania

**Tabela wyników do wypełnienia:**

| Konfiguracja | Avg wait czytelnik | Avg wait pisarz | Max wait pisarz |
|-------------|-------------------|----------------|----------------|
| 5R / 1W, `std::shared_mutex` | ___ ms | ___ ms | ___ ms |
| 10R / 1W, `std::shared_mutex` | ___ ms | ___ ms | ___ ms |
| 20R / 1W, `std::shared_mutex` | ___ ms | ___ ms | ___ ms |
| 5R / 1W, `std::mutex` (baseline) | ___ ms | ___ ms | ___ ms |

**Pytania do odpowiedzi:**
- Jak rośnie czas oczekiwania pisarza gdy zwiększamy liczbę czytelników?
- Czy `std::shared_mutex` jest zawsze lepszy od `std::mutex`?
- Czy zaobserwowałeś zagłodzenie? Przy jakiej konfiguracji?

**Szkielet:**

```cpp
#include <iostream>
#include <thread>
#include <shared_mutex>
#include <vector>
#include <chrono>
#include <atomic>
#include <algorithm>
#include <numeric>

struct ThreadStats {
    std::vector<double> wait_times_us;
    int operations = 0;

    double avg() const {
        if (wait_times_us.empty()) return 0;
        return std::accumulate(wait_times_us.begin(), wait_times_us.end(), 0.0)
               / wait_times_us.size();
    }
    double max() const {
        if (wait_times_us.empty()) return 0;
        return *std::max_element(wait_times_us.begin(), wait_times_us.end());
    }
};

std::shared_mutex rw_mutex;
int shared_data = 0;

void reader_bench(ThreadStats& stats, int num_reads) {
    for (int i = 0; i < num_reads; ++i) {
        auto start = std::chrono::steady_clock::now();

        // TODO: zablokuj shared_lock

        auto after_lock = std::chrono::steady_clock::now();
        volatile int tmp = shared_data;
        (void)tmp;

        // TODO: odblokuj

        double wait_us = std::chrono::duration<double, std::micro>(
            after_lock - start).count();
        stats.wait_times_us.push_back(wait_us);
        stats.operations++;
    }
}

void writer_bench(ThreadStats& stats, int num_writes) {
    for (int i = 0; i < num_writes; ++i) {
        auto start = std::chrono::steady_clock::now();

        // TODO: zablokuj unique_lock

        auto after_lock = std::chrono::steady_clock::now();
        shared_data++;

        // TODO: odblokuj

        double wait_us = std::chrono::duration<double, std::micro>(
            after_lock - start).count();
        stats.wait_times_us.push_back(wait_us);
        stats.operations++;
    }
}

int main(int argc, char* argv[]) {
    int num_readers = (argc > 1) ? std::atoi(argv[1]) : 5;
    int num_writers = (argc > 2) ? std::atoi(argv[2]) : 1;

    // TODO: stworzyć wątki, zebrać statystyki, wypisać wyniki

    return 0;
}
```

---

### Ćwiczenie 3: Zaimplementuj writers-preference od zera 🛠️

**Zadanie:** Zaimplementuj klasę `WritersPreferenceRWLock` z sekcji 5.2, ale
**bez podglądania rozwiązania**. Wymagania:

1. Metody: `read_lock()`, `read_unlock()`, `write_lock()`, `write_unlock()`
2. Wielu czytelników jednocześnie
3. Pisarz ma priorytet — gdy pisarz czeka, nowi czytelnicy nie wchodzą
4. Brak zagłodzenia pisarzy

**Test poprawności (uruchom i sprawdź):**

```cpp
#include <cassert>
#include <atomic>

// Twoja implementacja:
// class WritersPreferenceRWLock { ... };

WritersPreferenceRWLock rw;
std::atomic<int> concurrent_readers{0};
std::atomic<int> max_concurrent_readers{0};
std::atomic<bool> writer_was_concurrent{false};
int shared_value = 0;

void test_reader(int) {
    for (int i = 0; i < 1000; ++i) {
        rw.read_lock();

        int r = ++concurrent_readers;
        // Zapamiętaj max jednoczesnych czytelników
        int expected = max_concurrent_readers.load();
        while (r > expected &&
               !max_concurrent_readers.compare_exchange_weak(expected, r));

        volatile int tmp = shared_value;  // odczyt
        (void)tmp;

        --concurrent_readers;
        rw.read_unlock();
    }
}

void test_writer(int id) {
    for (int i = 0; i < 100; ++i) {
        rw.write_lock();

        // Podczas pisania NIE powinno być czytelników
        if (concurrent_readers.load() > 0) {
            writer_was_concurrent = true;  // 💥 BŁĄD!
        }
        shared_value = id * 1000 + i;

        rw.write_unlock();
    }
}

int main() {
    std::vector<std::thread> threads;
    for (int i = 0; i < 8; ++i) threads.emplace_back(test_reader, i);
    for (int i = 0; i < 2; ++i) threads.emplace_back(test_writer, i);
    for (auto& t : threads) t.join();

    assert(!writer_was_concurrent && "Pisarz był aktywny równocześnie z czytelnikiem!");
    assert(max_concurrent_readers > 1 && "Czytelnicy nigdy nie działali równolegle!");

    std::cout << "✅ Testy przeszły!\n";
    std::cout << "Max jednoczesnych czytelników: " << max_concurrent_readers << "\n";
}
```

---

### Ćwiczenie 4: Thread-safe sorted list 🧩

Zaimplementuj thread-safe listę posortowaną z operacjami:

```cpp
class ThreadSafeSortedList {
public:
    void insert(int value);                    // pisarz
    bool contains(int value) const;            // czytelnik
    std::vector<int> snapshot() const;         // czytelnik — kopia całej listy
    bool remove(int value);                    // pisarz
    size_t size() const;                       // czytelnik

    // BONUS: range query — zwraca elementy z zakresu [lo, hi]
    std::vector<int> range(int lo, int hi) const;  // czytelnik
};
```

**Wymagania:**
- Użyj `std::shared_mutex`
- `contains()` i `snapshot()` mogą działać równolegle
- `insert()` i `remove()` wymagają wyłącznego dostępu
- Napisz test wielowątkowy demonstrujący poprawność

**Podpowiedź:** Wewnętrznie użyj `std::set<int>` lub `std::vector<int>` (posortowany).

---

### Ćwiczenie 5: Porównanie wydajności 🏎️

Zaimplementuj **trzy wersje** tej samej struktury (np. `ThreadSafeCache` z sekcji 7.1):

1. **Wersja A:** `std::mutex` (exclusive dla wszystkich operacji)
2. **Wersja B:** `std::shared_mutex` (`shared_lock` dla `get()`, `unique_lock` dla `put()`)
3. **Wersja C:** RCU-like (sekcja 7.2)

**Benchmark:**
- Wypełnij cache 10000 elementami
- Uruchom N wątków robiących losowe operacje (95% get, 5% put)
- Mierz operacje/sekundę
- Testuj dla N = 1, 2, 4, 8, 16

**Tabela wyników:**

| Wątki | mutex (ops/s) | shared_mutex (ops/s) | RCU-like (ops/s) |
|:-----:|:------------:|:-------------------:|:----------------:|
| 1 | | | |
| 2 | | | |
| 4 | | | |
| 8 | | | |
| 16 | | | |

**Pytania:**
- Przy ilu wątkach `shared_mutex` zaczyna wygrywać z `mutex`?
- Czy RCU jest zawsze najszybszy? Jaka jest jego wada?
- Co się stanie gdy zmienisz proporcję na 50% get / 50% put?

---

### Ćwiczenie 6: Upgrade locka — wzorzec check-then-act 🔄

Zaimplementuj funkcję `get_or_compute()` dla cache, która:

1. Sprawdza czy klucz istnieje (shared_lock)
2. Jeśli tak → zwraca wartość
3. Jeśli nie → oblicza wartość (drogie!), zapisuje do cache (unique_lock), zwraca

```cpp
class ComputeCache {
    mutable std::shared_mutex mutex_;
    std::unordered_map<int, int> cache_;

public:
    // Zaimplementuj t�� funkcję:
    int get_or_compute(int key, std::function<int(int)> compute_fn) {
        // TODO:
        // 1. shared_lock → sprawdź cache
        // 2. Jeśli znaleziono → zwróć
        // 3. Zwolnij shared_lock
        // 4. unique_lock → sprawdź PONOWNIE (ktoś mógł dodać w międzyczasie!)
        // 5. Jeśli nadal brak → oblicz i wstaw
    }
};
```

**Test:**
```cpp
ComputeCache cache;
std::atomic<int> compute_count{0};

auto expensive = [&](int key) -> int {
    compute_count++;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    return key * key;
};

// 10 wątków jednocześnie prosi o ten sam klucz:
std::vector<std::thread> threads;
for (int i = 0; i < 10; ++i) {
    threads.emplace_back([&] {
        int result = cache.get_or_compute(42, expensive);
        assert(result == 42 * 42);
    });
}
for (auto& t : threads) t.join();

// compute powinno być wywołane NIEWIELE razy (idealnie 1, w praktyce kilka)
std::cout << "Compute wywołane: " << compute_count << " razy\n";
assert(compute_count <= 10);  // na pewno ≤ 10
// Czy udało Ci się osiągnąć compute_count == 1?
```

**Pytanie bonusowe:** Czy da się zagwarantować `compute_count == 1`?
Jakie są trade-offy? (Podpowiedź: call coalescing / `std::call_once`)

---

### Ćwiczenie 7: Dining Readers-Writers (kreatywne!) 🎭

Połącz problem czytelników-pisarzy z problemem ucztujących filozofów:

Masz **5 książek** ułożonych w okrąg. Każdy **filozof-czytelnik** potrzebuje
**dwóch sąsiednich książek** żeby czytać. Każdy **filozof-pisarz** potrzebuje
**jednej książki** żeby pisać (exclusive).

```
        📕 Książka 0
       /            \
  📗 Książka 4    📘 Książka 1
      |              |
  📙 Książka 3 ── 📒 Książka 2


Czytelnik i: potrzebuje shared_lock na książkę[i] ORAZ książkę[(i+1)%5]
Pisarz i:    potrzebuje unique_lock na książkę[i]
```

**Zadanie:**
1. Zaimplementuj rozwiązanie bez deadlocka
2. Czy `std::scoped_lock` działa z `std::shared_mutex` w trybie shared? (Nie!)
   Jak to rozwiązać?
3. Czy możliwe jest zagłodzenie? W jakim scenariuszu?

---

## 10. Podsumowanie

### Kluczowe wnioski

1. **`std::mutex` to czasem za dużo** — jeśli masz wielu czytelników, blokujesz ich niepotrzebnie
2. **`std::shared_mutex`** rozwiązuje to: `shared_lock` (czytelnicy) vs `unique_lock` (pisarze)
3. **Zagłodzenie jest realne** — i zależy od wariantu (readers-pref / writers-pref / fair)
4. **Standard C++ nie gwarantuje fairness** — zachowanie `std::shared_mutex` jest implementation-defined
5. **Upgrade shared→exclusive to deadlock** — zawsze zwalniaj, potem blokuj ponownie
6. **Mierz zanim optymalizujesz** — `shared_mutex` nie zawsze jest szybszy od `mutex`

### Kiedy co wybrać?

```
Potrzebujesz synchronizacji?
│
├─ Tylko pisarze (lub mało czytelników)?
│  └─ std::mutex ✅
│
├─ Dużo czytelników, mało pisarzy?
│  ├─ Sekcja krytyczna > 1μs?
│  │  └─ std::shared_mutex ✅
│  └─ Sekcja krytyczna < 1μs?
│     └─ Zmierz! Może std::mutex wystarczy
│
├─ Potrzebujesz gwarancji fairness?
│  └─ Implementuj ręcznie (fair FIFO) lub użyj kolejki
│
└─ Ekstremalnie dużo odczytów, rzadkie zapisy?
   └─ Rozważ RCU / atomic shared_ptr (C++20)
```

---

## Literatura

- Courtois, Heymans, Parnas, *Concurrent Control with Readers and Writers*, CACM 1971
- C++ Reference: [std::shared_mutex](https://en.cppreference.com/w/cpp/thread/shared_mutex)
- A. Williams, *C++ Concurrency in Action*, 2nd ed., rozdział 3.3
- Fedor Pikus, *C++ Concurrency in Action: Practical Multithreading* (CppCon talks)
- P. McKenney, *Is Parallel Programming Hard?* — rozdziały o RCU
