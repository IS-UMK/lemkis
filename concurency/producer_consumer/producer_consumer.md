
# Producent-Konsument (Producer-Consumer)
## Notatki do wykładu/ćwiczeń z programowania współbieżnego w C++

---

## 1. Wprowadzenie — o czym jest ten schemat?

**Problem producenta-konsumenta** to jeden z klasycznych problemów  
synchronizacji, sformułowany
po raz pierwszy przez Edsgera Dijkstrę (1965).

### Scenariusz
- **Producent** — wątek generujący dane i umieszczający je w buforze
- **Konsument** — wątek pobierający dane z bufora i je przetwarzający
- **Bufor** — współdzielona struktura danych o ograniczonej (lub  
nieograniczonej) pojemności

### Przykłady z życia
| Producent | Bufor | Konsument |
|-----------|-------|-----------|
| Serwer HTTP przyjmujący requesty | Kolejka zadań | Wątki workera  
przetwarzające requesty |
| Wątek odczytujący dane z sieci | Ring buffer | Wątek dekodujący dane |
| Czujnik (IoT) generujący pomiary | Bufor kołowy | Wątek zapisujący  
do bazy danych |
| GUI rejestrujące kliknięcia | Kolejka zdarzeń | Wątek logiki aplikacji |
| `std::async` produkujący wynik | `std::future`/`std::promise` |  
Wątek oczekujący na wynik |

---

## 2. Warianty problemu

### 2.1 Bufor nieograniczony (unbounded buffer)
- Producent **nigdy nie czeka** — zawsze może dodać element
- Konsument czeka, gdy bufor jest pusty
- Prostszy, ale niebezpieczny — niekontrolowane zużycie pamięci

### 2.2 Bufor ograniczony (bounded buffer) ← **nasz główny przypadek**
- Producent **czeka**, gdy bufor jest pełny
- Konsument **czeka**, gdy bufor jest pusty
- Bardziej realistyczny i bezpieczny

### 2.3 Warianty liczności
- 1 producent, 1 konsument (1P–1C)
- 1 producent, N konsumentów (1P–NC)
- M producentów, 1 konsument (MP–1C)
- M producentów, N konsumentów (MP–NC) ← **najbardziej ogólny**

---

## 3. Naiwna implementacja — co może pójść nie tak?

### 3.1 Wersja bez synchronizacji (❌ BŁĘDNA)

```cpp
#include <iostream>
#include <thread>
#include <queue>

std::queue<int> bufor;
const int MAX_SIZE = 5;

void producent() {
     for (int i = 0; i < 20; ++i) {
         while (bufor.size() >= MAX_SIZE) {
             // busy wait — czekamy aż się zwolni miejsce
         }
         bufor.push(i); // ❌ DATA RACE!
         std::cout << "Wyprodukowano: " << i << "\n"; // ❌ DATA RACE na cout!
     }
}

void konsument() {
     for (int i = 0; i < 20; ++i) {
         while (bufor.empty()) {
             // busy wait — czekamy aż coś się pojawi
         }
         int val = bufor.front(); // ❌ DATA RACE!
         bufor.pop();             // ❌ DATA RACE!
         std::cout << "Skonsumowano: " << val << "\n";
     }
}

int main() {
     std::thread p(producent);
     std::thread k(konsument);
     p.join();
     k.join();
}
```

### Problemy tej wersji

| # | Problem | Konsekwencja |
|---|---------|-------------|
| 1 | **Data race** na `std::queue` | Undefined Behavior — program  
może zrobić dosłownie cokolwiek |
| 2 | **Data race** na `std::cout` | Pomieszane znaki na wyjściu |
| 3 | **Busy waiting** (aktywne czekanie) | 100% CPU, marnowanie zasobów |
| 4 | **Brak gwarancji widoczności** | Kompilator/CPU może  
zoptymalizować pętlę `while` w nieskończoną |

> **⚠️ Ważne:** Wyścig danych (data race) na **dowolnym** nieatomowym  
> obiekcie to
> **Undefined Behavior** w sensie standardu C++ — nie „czasem działa źle",
> lecz program jest **całkowicie nieprzewidywalny**.

### 3.2 Wersja z mutex, ale bez condition variable (⚠️ DZIAŁA, ALE  
NIEOPTYMALNA)

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <queue>

std::queue<int> bufor;
const int MAX_SIZE = 5;
std::mutex mtx;

void producent() {
     for (int i = 0; i < 20; ++i) {
         while (true) {
             std::lock_guard<std::mutex> lock(mtx);
             if (bufor.size() < MAX_SIZE) {
                 bufor.push(i);
                 std::cout << "Wyprodukowano: " << i << "\n";
                 break;
             }
             // ⚠️ zwalniamy mutex i natychmiast próbujemy ponownie =  
spin lock!
         }
     }
}

void konsument() {
     for (int i = 0; i < 20; ++i) {
         while (true) {
             std::lock_guard<std::mutex> lock(mtx);
             if (!bufor.empty()) {
                 int val = bufor.front();
                 bufor.pop();
                 std::cout << "Skonsumowano: " << val << "\n";
                 break;
             }
         }
     }
}

int main() {
     std::thread p(producent);
     std::thread k(konsument);
     p.join();
     k.join();
}
```

### Analiza

| Aspekt | Ocena |
|--------|-------|
| Poprawność | ✅ Brak data race'ów |
| Wydajność | ❌ Busy waiting — ciągłe lock/unlock muteksa |
| Zużycie CPU | ❌ Wysokie — wątek „kręci się" w pętli |
| Opóźnienia | ❌ Nieprzewidywalne — konsument może „nie zdążyć" złapać  
mutexa |
| Fairness | ❌ Brak gwarancji — wątek może głodować |

> **Wniosek:** Potrzebujemy mechanizmu, który pozwoli wątkowi  
> **zasnąć** i zostać
> **obudzonym**, gdy warunek będzie spełniony → `std::condition_variable`.

---

## 4. Poprawna implementacja z `std::condition_variable`

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <vector>
#include <atomic>
#include <cassert>

template<typename T>
class BoundedBuffer {
     std::queue<T> queue_;
     const size_t max_size_;

     std::mutex mutex_;
     std::condition_variable cv_not_full_;   // producenci czekają na to
     std::condition_variable cv_not_empty_;  // konsumenci czekają na to

public:
     explicit BoundedBuffer(size_t max_size) : max_size_(max_size) {
         assert(max_size > 0);
     }

     // Wstawia element do bufora. Blokuje, jeśli bufor jest pełny.
     void push(const T& item) {
         std::unique_lock<std::mutex> lock(mutex_);

         // Czekaj aż bufor nie będzie pełny
         //
         // wait() robi trzy rzeczy:
         //   1. Sprawdza predykat — jeśli true, nie czeka
         //   2. Jeśli false: ATOMOWO zwalnia mutex i usypia wątek
         //   3. Po obudzeniu: ponownie blokuje mutex i sprawdza predykat
         //
         // Dlaczego predykat, a nie zwykłe wait()?
         //   → Ochrona przed SPURIOUS WAKEUP (fałszywe obudzenie)
         //   → System operacyjny MOŻE obudzić wątek bez notify!
         cv_not_full_.wait(lock, [this] {
             return queue_.size() < max_size_;
         });

         queue_.push(item);

         // Budzenie JEDNEGO czekającego konsumenta
         cv_not_empty_.notify_one();
     }

     // Pobiera element z bufora. Blokuje, jeśli bufor jest pusty.
     T pop() {
         std::unique_lock<std::mutex> lock(mutex_);

         // Czekaj aż bufor nie będzie pusty
         cv_not_empty_.wait(lock, [this] {
             return !queue_.empty();
         });

         T item = std::move(queue_.front());
         queue_.pop();

         // Budzenie JEDNEGO czekającego producenta
         cv_not_full_.notify_one();

         return item;
     }
};

int main() {
     BoundedBuffer<int> bufor(5);
     std::atomic<int> suma_produced{0};
     std::atomic<int> suma_consumed{0};

     const int NUM_PRODUCERS = 3;
     const int NUM_CONSUMERS = 2;
     const int ITEMS_PER_PRODUCER = 100;

     // --- Producenci ---
     std::vector<std::thread> producers;
     for (int id = 0; id < NUM_PRODUCERS; ++id) {
         producers.emplace_back([&bufor, &suma_produced, id] {
             for (int i = 0; i < ITEMS_PER_PRODUCER; ++i) {
                 int value = id * ITEMS_PER_PRODUCER + i;
                 bufor.push(value);
                 suma_produced += value;
             }
         });
     }

     // --- Konsumenci ---
     const int TOTAL_ITEMS = NUM_PRODUCERS * ITEMS_PER_PRODUCER;
     std::atomic<int> consumed_count{0};

     std::vector<std::thread> consumers;
     for (int id = 0; id < NUM_CONSUMERS; ++id) {
         consumers.emplace_back([&bufor, &suma_consumed,  
&consumed_count, TOTAL_ITEMS] {
             while (true) {
                 int my_count = consumed_count.fetch_add(1);
                 if (my_count >= TOTAL_ITEMS) break;

                 int value = bufor.pop();
                 suma_consumed += value;
             }
         });
     }

     for (auto& t : producers) t.join();
     for (auto& t : consumers) t.join();

     std::cout << "Suma wyprodukowana: " << suma_produced << "\n";
     std::cout << "Suma skonsumowana:  " << suma_consumed << "\n";
     assert(suma_produced == suma_consumed);
     std::cout << "✅ Wszystkie elementy przetworzone poprawnie!\n";
}
```

---

## 5. Analiza krok po kroku: co robi `condition_variable::wait()`?

```
Wątek producenta                          Wątek konsumenta
─────────────────                         ─────────────────
lock(mutex)                               lock(mutex)
   │                                         │
   ▼                                         ▼
   bufor pełny?                              bufor pusty?
   ├─ NIE → push + notify ─────────────┐    ├─ NIE → pop + notify
   │                                    │    │
   └─ TAK → cv.wait():                 │    └─ TAK → cv.wait():
            ┌──────────────────┐       │             ┌──────────────────┐
            │ 1. UNLOCK mutex  │       │             │ 1. UNLOCK mutex  │
            │ 2. Wątek ŚPII    │ ◄─────┘             │ 2. Wątek ŚPII    │
            │ 3. (notify)      │                     │ 3. (notify)      │
            │ 4. LOCK mutex    │                     │ 4. LOCK mutex    │
            │ 5. Sprawdź pred. │                     │ 5. Sprawdź pred. │
            └──────────────────┘                     └──────────────────┘
                     │                                        ���
                     ▼                                        ▼
             predykat true?                           predykat true?
             ├─ TAK → kontynuuj                       ├─ TAK → kontynuuj
             └─ NIE → wróć do ŚPII                    └─ NIE → wróć do ŚPII
```

### Kluczowe obserwacje

1. **Atomowość unlock+sleep** — gdyby to były dwie osobne operacje, mógłby
    wystąpić **lost wakeup** (notify przychodzi między unlock a sleep)

2. **Predykat jest KONIECZNY** z dwóch powodów:
    - **Spurious wakeup** — OS może obudzić wątek bez powodu
    - **Stolen wakeup** — inny wątek mógł „ukraść" element między  
notify a ponownym lock

3. **`unique_lock`** (nie `lock_guard`!) — bo `wait()` musi móc unlock/lock

---

## 6. Zagrożenia i pułapki

### 6.1 Lost Wakeup (utracone powiadomienie)

```cpp
// ❌ ŹLE — możliwy lost wakeup!
void push_zle(const T& item) {
     {
         std::lock_guard<std::mutex> lock(mutex_);
         queue_.push(item);
     }
     // ← Tutaj konsument może: lock → sprawdzić → pop → unlock
     //   a my dopiero teraz robimy notify (do nikogo!)
     cv_not_empty_.notify_one();
     // To akurat DZIAŁA, bo konsument sprawdza predykat.
     // ALE: gdyby konsument czekał BEZ predykatu — notify byłoby stracone.
}
```

> **Zasada:** `notify_one()`/`notify_all()` **nie buforuje** powiadomień.
> Jeśli żaden wątek nie czeka w momencie notify — powiadomienie przepada.
> Dlatego **zawsze** używamy `wait()` z predykatem.

### 6.2 Spurious Wakeup (fałszywe obudzenie)

```cpp
// ❌ ŹLE — brak predykatu!
void pop_zle() {
     std::unique_lock<std::mutex> lock(mutex_);
     cv_not_empty_.wait(lock);  // ← może się obudzić bez notify!
     // queue_ może nadal być puste!
     T item = queue_.front();   // 💥 UB jeśli pusta kolejka
     queue_.pop();
}

// ✅ DOBRZE — z predykatem
void pop_ok() {
     std::unique_lock<std::mutex> lock(mutex_);
     cv_not_empty_.wait(lock, [this] { return !queue_.empty(); });
     // Gwarancja: queue_ nie jest puste
}
```

### 6.3 Deadlock przy wielu zasobach

```cpp
// ❌ ŹLE — dwa muteksy, niespójna kolejność
void transfer(BoundedBuffer& a, BoundedBuffer& b) {
     std::lock_guard<std::mutex> lock_a(a.mutex_);  // lock A
     std::lock_guard<std::mutex> lock_b(b.mutex_);  // lock B — DEADLOCK!
     // Jeśli inny wątek robi transfer(b, a) → lock B, potem lock A
}

// ✅ DOBRZE — std::scoped_lock
void transfer(BoundedBuffer& a, BoundedBuffer& b) {
     std::scoped_lock lock(a.mutex_, b.mutex_);  // atomowy lock obu
     // ...
}
```

### 6.4 `notify_one()` vs `notify_all()` — kiedy co?

| Metoda | Budzi | Kiedy używać |
|--------|-------|-------------|
| `notify_one()` | Jeden wątek | Gdy **jeden** czekający wątek może  
obsłużyć zdarzenie |
| `notify_all()` | Wszystkie wątki | Gdy **warunek się zmienił** i  
wielu czekających powinno ponownie sprawdzić |

```cpp
// Sytuacja: 1 element dodany → wystarczy obudzić 1 konsumenta
cv_not_empty_.notify_one();  // ✅ optymalne

// Sytuacja: bufor zamykamy, wszyscy muszą się obudzić i zakończyć
cv_not_empty_.notify_all();  // ✅ konieczne
```

**Pułapka z `notify_one()`:** Jeśli obudzony wątek nie może kontynuować
(np. ponownie zasypia z innego powodu), **nikt inny nie zostanie obudzony**!
W razie wątpliwości → `notify_all()`.

### 6.5 Zagłodzenie (starvation)

Ani `std::mutex`, ani `std::condition_variable` nie gwarantują **fairness**.
Wątek może być ciągle pomijany. Rozwiązania:
- Użycie kolejki FIFO do zarządzania oczekującymi
- Ticketing (każdy wątek dostaje numerek)

---

## 7. Optymalizacje

### 7.1 Zmniejszenie zakresu sekcji krytycznej

```cpp
// ⚠️ NIEOPTYMALNE — notify pod mutexem
void push(const T& item) {
     std::unique_lock<std::mutex> lock(mutex_);
     cv_not_full_.wait(lock, [this] { return queue_.size() < max_size_; });
     queue_.push(item);
     cv_not_empty_.notify_one();  // ← wątek obudzony natychmiast  
blokuje się na mutex!
}

// ✅ LEPIEJ — notify poza mutexem
void push(const T& item) {
     {
         std::unique_lock<std::mutex> lock(mutex_);
         cv_not_full_.wait(lock, [this] { return queue_.size() < max_size_; });
         queue_.push(item);
     }   // ← mutex zwolniony
     cv_not_empty_.notify_one();  // obudzony wątek od razu może  
zablokować mutex
}
```

> **Uwaga:** Ta optymalizacja jest **subtelna**. W wielu implementacjach OS
> wątek obudzony przez `notify` jest przenoszony z kolejki CV do  
> kolejki muteksa,
> więc różnica może być minimalna. Ale w teorii — unikamy niepotrzebnego
> „obudź → natychmiast zablokuj".

### 7.2 Bufor kołowy zamiast `std::queue`

`std::queue<T>` wewnętrznie używa `std::deque`, który alokuje pamięć  
dynamicznie.
W scenariuszach o wysokiej przepustowości — bufor kołowy (ring buffer):

```cpp
template<typename T>
class RingBuffer {
     std::vector<T> data_;
     size_t head_ = 0;          // indeks do zapisu (producent)
     size_t tail_ = 0;          // indeks do odczytu (konsument)
     size_t count_ = 0;         // liczba elementów
     size_t capacity_;

     std::mutex mutex_;
     std::condition_variable cv_not_full_;
     std::condition_variable cv_not_empty_;

public:
     explicit RingBuffer(size_t capacity)
         : data_(capacity), capacity_(capacity) {}

     void push(const T& item) {
         std::unique_lock<std::mutex> lock(mutex_);
         cv_not_full_.wait(lock, [this] { return count_ < capacity_; });

         data_[head_] = item;
         head_ = (head_ + 1) % capacity_;
         ++count_;

         cv_not_empty_.notify_one();
     }

     T pop() {
         std::unique_lock<std::mutex> lock(mutex_);
         cv_not_empty_.wait(lock, [this] { return count_ > 0; });

         T item = std::move(data_[tail_]);
         tail_ = (tail_ + 1) % capacity_;
         --count_;

         cv_not_full_.notify_one();
         return item;
     }
};
```

**Zalety:**
- Brak alokacji dynamicznej po inicjalizacji
- Cache-friendly — ciągły blok pamięci
- Przewidywalne zużycie pamięci

### 7.3 Lock-free ring buffer (1P-1C) — zaawansowane

Dla przypadku **dokładnie 1 producent i 1 konsument** możemy całkowicie
wyeliminować mutex:

```cpp
#include <atomic>
#include <vector>
#include <optional>

template<typename T>
class SPSCRingBuffer {
     // Single Producer, Single Consumer
     std::vector<T> data_;
     const size_t capacity_;

     // Producent pisze TYLKO do head_, konsument pisze TYLKO do tail_
     // Dzięki temu nie ma wyścigu — każdy atomic ma jednego pisarza
     alignas(64) std::atomic<size_t> head_{0};  // alignas(64) →  
osobna linia cache
     alignas(64) std::atomic<size_t> tail_{0};  // unikamy false sharing

public:
     explicit SPSCRingBuffer(size_t capacity)
         : data_(capacity), capacity_(capacity) {}

     // Wywoływany TYLKO przez producenta
     bool try_push(const T& item) {
         size_t head = head_.load(std::memory_order_relaxed);
         size_t next = (head + 1) % capacity_;

         if (next == tail_.load(std::memory_order_acquire)) {
             return false;  // bufor pełny
         }

         data_[head] = item;
         head_.store(next, std::memory_order_release);
         return true;
     }

     // Wywoływany TYLKO przez konsumenta
     std::optional<T> try_pop() {
         size_t tail = tail_.load(std::memory_order_relaxed);

         if (tail == head_.load(std::memory_order_acquire)) {
             return std::nullopt;  // bufor pusty
         }

         T item = std::move(data_[tail]);
         tail_.store((tail + 1) % capacity_, std::memory_order_release);
         return item;
     }
};
```

**Kiedy warto:**
- Ultra-niska latencja (finanse, audio, gry)
- Dokładnie 1 producent i 1 konsument (SPSC)
- Pomiary wykazały, że mutex jest wąskim gardłem

**Kiedy NIE warto:**
- Wielu producentów lub konsumentów
- Kod musi być czytelny i łatwy w utrzymaniu
- Nie zmierzono, że mutex jest problemem ← **premature optimization!**

### 7.4 Batch processing — przetwarzanie wsadowe

Zamiast push/pop pojedynczych elementów — przenosimy całe porcje:

```cpp
// Producent wstawia wiele elementów naraz
void push_batch(const std::vector<T>& items) {
     std::unique_lock<std::mutex> lock(mutex_);
     for (const auto& item : items) {
         cv_not_full_.wait(lock, [this] { return queue_.size() < max_size_; });
         queue_.push(item);
         cv_not_empty_.notify_one();
     }
}

// Konsument pobiera wszystko co jest dostępne
std::vector<T> drain() {
     std::unique_lock<std::mutex> lock(mutex_);
     cv_not_empty_.wait(lock, [this] { return !queue_.empty(); });

     std::vector<T> result;
     while (!queue_.empty()) {
         result.push_back(std::move(queue_.front()));
         queue_.pop();
     }
     // Budzimy WSZYSTKICH producentów — bufor jest teraz pusty
     cv_not_full_.notify_all();
     return result;
}
```

**Zaleta:** Mniej operacji lock/unlock na element.

---

## 8. Porównanie podejść — tabela podsumowująca

| Podejście | Poprawność | CPU | Latencja | Przepustowość | Złożoność kodu |
|-----------|:----------:|:---:|:--------:|:-------------:|:--------------:|
| Bez synchronizacji | ❌ UB | ❌ | — | — | ⭐ |
| Mutex + busy wait | ✅ | ❌ | ❌ | ❌ | ⭐�� |
| Mutex + CV (`std::queue`) | ✅ | ✅ | ✅ | ✅ | ⭐⭐⭐ |
| Mutex + CV (ring buffer) | ✅ | ✅ | ✅✅ | ✅✅ | ⭐⭐⭐ |
| Lock-free SPSC | ✅* | ✅✅ | ✅✅✅ | ✅✅✅ | ⭐⭐⭐⭐⭐ |
| Batch processing | ✅ | ✅ | ✅ | ✅✅✅ | ⭐⭐⭐ |

*\* Poprawność lock-free kodu jest trudna do zweryfikowania i przetestowania.*

---

## 9. Wzorce graceful shutdown

Jak poprawnie zakończyć producenta-konsumenta?

### 9.1 Podejście z flagą `done`

```cpp
template<typename T>
class BoundedBufferWithShutdown {
     std::queue<T> queue_;
     size_t max_size_;
     std::mutex mutex_;
     std::condition_variable cv_not_full_;
     std::condition_variable cv_not_empty_;
     bool shutdown_ = false;

public:
     explicit BoundedBufferWithShutdown(size_t max_size) :  
max_size_(max_size) {}

     // Zwraca false jeśli bufor został zamknięty
     bool push(const T& item) {
         std::unique_lock<std::mutex> lock(mutex_);
         cv_not_full_.wait(lock, [this] {
             return queue_.size() < max_size_ || shutdown_;
         });
         if (shutdown_) return false;

         queue_.push(item);
         cv_not_empty_.notify_one();
         return true;
     }

     // Zwraca std::nullopt jeśli bufor zamknięty i pusty
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
         // Budzimy WSZYSTKICH — zarówno producentów jak i konsumentów
         cv_not_full_.notify_all();
         cv_not_empty_.notify_all();
     }
};
```

### 9.2 Podejście z „trucizną" (poison pill / sentinel)

```cpp
// Producent na końcu wstawia specjalną wartość:
bufor.push(-1);  // -1 = "koniec pracy"

// Konsument:
while (true) {
     int val = bufor.pop();
     if (val == -1) {
         bufor.push(-1);  // przekaż dalej do innych konsumentów!
         break;
     }
     process(val);
}
```

**Porównanie:**

| Podejście | Zalety | Wady |
|-----------|--------|------|
| Flaga `shutdown` | Czysty API, działa z każdym typem T | Dodatkowa  
logika w push/pop |
| Poison pill | Prostota implementacji | Wymaga „specjalnej wartości",  
nie działa z każdym typem |
| `std::stop_token` (C++20) | Standardowy mechanizm | Wymaga C++20 |

---

## 10. Ćwiczenia dla studentów

### Ćwiczenie 1: Znajdź błąd 🔍
Czy poniższy kod jest poprawny? Jeśli nie — jaki błąd i jak naprawić?

```cpp
void push(const T& item) {
     std::lock_guard<std::mutex> lock(mutex_);
     cv_not_full_.wait(/* ??? */);  // Czy lock_guard zadziała z wait()?
     queue_.push(item);
     cv_not_empty_.notify_one();
}
```

<details>
<summary>Odpowiedź</summary>

`std::condition_variable::wait()` wymaga `std::unique_lock`, nie  
`std::lock_guard`,
ponieważ musi móc zwolnić i ponownie zablokować mutex.
`lock_guard` nie udostępnia metod `lock()`/`unlock()`.
Kod się **nie skompiluje**.

</details>

### Ćwiczenie 2: Symulacja 🧪
Zaimplementuj system z:
- 3 producentami (każdy produkuje 1000 elementów)
- 2 konsumentami
- Buforem o rozmiarze 10
- Zmierz czas wykonania
- Porównaj z buforem o rozmiarze 1 i 1000 — jak rozmiar bufora wpływa  
na wydajność?

### Ćwiczenie 3: Pipeline 🔗
Zbuduj potok przetwarzania z trzema etapami:

```
[Generator] → bufor1 → [Transformer] → bufor2 → [Writer]
      ↓                      ↓                       ↓
   produkuje            przetwarza              zapisuje
   liczby 1..N          (x * x)                 na ekran
```

Każdy etap to osobny wątek. Użyj `BoundedBuffer` jako buforów między etapami.

### Ćwiczenie 4: Analiza wydajności 📊
Zbadaj wpływ parametrów na przepustowość (elementów/sekundę):

| Parametr do zbadania | Zakres wartości |
|---------------------|----------------|
| Rozmiar bufora | 1, 10, 100, 1000, 10000 |
| Liczba producentów | 1, 2, 4, 8 |
| Liczba konsumentów | 1, 2, 4, 8 |
| Czas produkcji elementu | 0μs, 1μs, 10μs, 100μs |
| Czas konsumpcji elementu | 0μs, 1μs, 10μs, 100μs |

### Ćwiczenie 5 (zaawansowane): Thread Pool 🏊
Zaimplementuj prostą pulę wątków opartą na wzorcu producent-konsument:

```cpp
class ThreadPool {
public:
     explicit ThreadPool(size_t num_threads);
     ~ThreadPool();  // graceful shutdown

     // Dodaje zadanie do kolejki
     void submit(std::function<void()> task);

private:
     std::vector<std::thread> workers_;
     BoundedBuffer<std::function<void()>> task_queue_;
};
```

---

## 11. Powiązania ze standardem C++

| Standard | Narzędzie | Znaczenie dla producenta-konsumenta |
|----------|-----------|-------------------------------------|
| C++11 | `std::mutex`, `std::condition_variable` | Podstawowa implementacja |
| C++11 | `std::unique_lock`, `std::lock_guard` | RAII zarządzanie muteksem |
| C++11 | `std::thread` | Tworzenie wątków |
| C++17 | `std::scoped_lock` | Bezpieczne blokowanie wielu muteksów |
| C++17 | `std::optional` | Sygnalizowanie braku wartości (graceful  
shutdown) |
| C++20 | `std::jthread` + `std::stop_token` | Kooperatywne anulowanie  
wątków |
| C++20 | `std::counting_semaphore` | Alternatywa dla CV w bounded buffer |
| C++20 | `std::latch`, `std::barrier` | Synchronizacja etapów pipeline'u |

---

## 12. Podsumowanie

### Kluczowe wnioski

1. **Sam `std::mutex` nie wystarczy** — potrzebujemy  
`condition_variable` aby unikać busy waiting
2. **Zawsze używaj predykatu** w `wait()` — ochrona przed spurious i  
lost wakeup
3. **`unique_lock`** jest wymagany dla `condition_variable` (nie `lock_guard`)
4. **Graceful shutdown** jest nietrywialny — zaplanuj go od początku
5. **Mierz zanim optymalizujesz** — lock-free jest rzadko potrzebne
6. **Producent-konsument jest wszędzie** — thread pools, pipelines,  
kolejki zdarzeń

### Reguły kciuka

> 🟢 Zacznij od `std::queue` + `mutex` + `condition_variable` — to  
> wystarczy w 95% przypadków.
>
> 🟡 Jeśli pomiary wykażą problem → ring buffer lub batch processing.
>
> 🔴 Lock-free tylko gdy masz **dowód**, że mutex jest wąskim gardłem,
> **i** masz dokładnie 1P-1C, **i** rozumiesz model pamięci C++.

---

## Literatura i zasoby

- C++ Reference:  
[std::condition_variable](https://en.cppreference.com/w/cpp/thread/condition_variable)
- A. Williams, *C++ Concurrency in Action*, 2nd ed. (Manning, 2019) —  
rozdziały 4–5
- H. Sutter, *Effective Concurrency* (seria artykułów na Dr. Dobb's)
- CppCon 2017: Fedor Pikus, *C++ atomics, from basic to advanced*
