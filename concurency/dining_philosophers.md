# Ucztujący Filozofowie (Dining Philosophers)
## Notatki do wykładu/ćwiczeń z programowania współbieżnego w C++

---

## 1. Wprowadzenie — o czym jest ten problem?

**Problem ucztujących filozofów** został sformułowany przez Edsgera Dijkstrę (1965)
i jest klasycznym przykładem ilustrującym problem **deadlocka** (zakleszczenia)
oraz strategie jego unikania.

### Scenariusz

Pięciu filozofów siedzi przy okrągłym stole. Każdy na przemian **myśli** i **je**.
Między każdą parą filozofów leży **jeden widelec** (łącznie 5 widelców).
Żeby jeść, filozof potrzebuje **dwóch widelców** — lewego i prawego.

```
                    Filozof 0
                   🧠
              🍴         🍴
          Widelec 4    Widelec 0
          
Filozof 4  🧠                🧠  Filozof 1
     
       Widelec 3                Widelec 1
          🍴                       🍴

           🧠
          Filozof 3            🧠  Filozof 2
                        🍴
                        Widelec 2
```

```
Filozof i:
  - lewy widelec:  widelec[i]
  - prawy widelec: widelec[(i + 1) % 5]

Filozof 0: widelce 0 i 1
Filozof 1: widelce 1 i 2
Filozof 2: widelce 2 i 3
Filozof 3: widelce 3 i 4
Filozof 4: widelce 4 i 0    ← dzieli widelec 0 z Filozofem 0!
```

### Algorytm filozofa

```
while (true) {
    myśl();
    weź_lewy_widelec();
    weź_prawy_widelec();
    jedz();
    odłóż_prawy_widelec();
    odłóż_lewy_widelec();
}
```

### Analogie z rzeczywistego świata

| Filozofowie | Widelce | Analogia |
|-------------|---------|----------|
| Wątki bazy danych | Locki na wierszach | Transakcja potrzebuje zablokować wiele wierszy |
| Wątki transferu bankowego | Locki na kontach | Transfer wymaga locka na koncie źródłowym I docelowym |
| Procesy drukarki | Urządzenia I/O | Proces potrzebuje drukarki I skanera jednocześnie |
| Wątki grafu | Locki na wierzchołkach | Modyfikacja krawędzi wymaga locka na obu wierzchołkach |

### Kluczowe pytania problemu

1. **Deadlock** — czy filozofowie mogą się wzajemnie zablokować?
2. **Starvation** — czy filozof może głodować w nieskończoność?
3. **Concurrency** — ile filozofów może jeść jednocześnie? (max 2 z 5)
4. **Fairness** — czy każdy filozof ma szansę zjeść?

---

## 2. Naiwna implementacja — DEADLOCK!

### 2.1 Wersja gwarantująca deadlock

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <array>
#include <chrono>

const int N = 5;
std::array<std::mutex, N> widelce;

void filozof(int id) {
    auto& lewy  = widelce[id];
    auto& prawy = widelce[(id + 1) % N];

    while (true) {
        // Myślenie
        std::cout << "Filozof " << id << " myśli...\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        // ❌ DEADLOCK: każdy bierze lewy, potem prawy
        lewy.lock();
        std::cout << "Filozof " << id << " wziął lewy widelec\n";

        // Dodajmy sleep żeby zwiększyć szansę na deadlock:
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        prawy.lock();
        std::cout << "Filozof " << id << " wziął prawy widelec\n";

        // Jedzenie
        std::cout << "Filozof " << id << " JE!\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(50));

        prawy.unlock();
        lewy.unlock();
    }
}

int main() {
    std::array<std::thread, N> filozofowie;
    for (int i = 0; i < N; ++i)
        filozofowie[i] = std::thread(filozof, i);
    for (auto& t : filozofowie)
        t.join();
}
```

### 2.2 Dlaczego dochodzi do deadlocka?

```
Każdy filozof bierze LEWY widelec:

Filozof 0: lock(widelec[0]) ✅
Filozof 1: lock(widelec[1]) ✅
Filozof 2: lock(widelec[2]) ✅
Filozof 3: lock(widelec[3]) ✅
Filozof 4: lock(widelec[4]) ✅

Teraz każdy próbuje wziąć PRAWY:

Filozof 0: lock(widelec[1]) → czeka na Filozofa 1 💤
Filozof 1: lock(widelec[2]) → czeka na Filozofa 2 💤
Filozof 2: lock(widelec[3]) → czeka na Filozofa 3 💤
Filozof 3: lock(widelec[4]) → czeka na Filozofa 4 💤
Filozof 4: lock(widelec[0]) → czeka na Filozofa 0 💤

        ┌─► Filozof 0 czeka na 1
        │   Filozof 1 czeka na 2
        │   Filozof 2 czeka na 3
        │   Filozof 3 czeka na 4
        └── Filozof 4 czeka na 0  ← CYKL! = DEADLOCK ☠️
```

### 2.3 Cztery warunki Coffmana (1971)

Jeśli Deadlock występuje to koniecznie **wszystkie cztery** warunki są spełnione jednocześnie:

| # | Warunek | W naszym problemie | Jak złamać? |
|---|---------|-------------------|-------------|
| 1 | **Mutual exclusion** — zasób jest wyłączny | Widelec może trzymać tylko 1 filozof | Trudne — to natura problemu |
| 2 | **Hold and wait** — trzymam i czekam na więcej | Trzymam lewy, czekam na prawy | Bierz oba naraz albo żadnego |
| 3 | **No preemption** — nikt nie może mi zabrać zasobu | Nikt nie zabiera widelca siłą | Pozwól na timeout/oddanie |
| 4 | **Circular wait** — cykliczne oczekiwanie | 0→1→2→3→4→0 | Uporządkuj zasoby (numeracja!) |

> **Żeby zapobiec deadlockowi, wystarczy złamać JEDEN z czterech warunków.**

---

## 3. Rozwiązanie 1: Hierarchia zasobów (łamie circular wait)

### Idea

Zamiast „każdy bierze najpierw lewy" → **każdy bierze najpierw widelec o niższym numerze**.

```
Filozof 0: min(0,1)=0, max(0,1)=1  → bierze 0, potem 1
Filozof 1: min(1,2)=1, max(1,2)=2  → bierze 1, potem 2
Filozof 2: min(2,3)=2, max(2,3)=3  → bierze 2, potem 3
Filozof 3: min(3,4)=3, max(3,4)=4  → bierze 3, potem 4
Filozof 4: min(4,0)=0, max(4,0)=4  → bierze 0, potem 4  ← ODWRÓCONA KOLEJNOŚĆ!
```

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <array>
#include <chrono>
#include <atomic>

const int N = 5;
std::array<std::mutex, N> widelce;
std::atomic<bool> running{true};

void filozof(int id) {
    int lewy_id  = id;
    int prawy_id = (id + 1) % N;

    // KLUCZ: zawsze blokuj mutex o niższym numerze najpierw!
    int pierwszy = std::min(lewy_id, prawy_id);
    int drugi    = std::max(lewy_id, prawy_id);

    while (running) {
        // Myślenie
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        widelce[pierwszy].lock();   // zawsze niższy numer najpierw
        widelce[drugi].lock();      // potem wyższy

        std::cout << "Filozof " << id << " je (widelce "
                  << pierwszy << "," << drugi << ")\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(50));

        widelce[drugi].unlock();
        widelce[pierwszy].unlock();
    }
}

int main() {
    std::array<std::thread, N> filozofowie;
    for (int i = 0; i < N; ++i)
        filozofowie[i] = std::thread(filozof, i);

    std::this_thread::sleep_for(std::chrono::seconds(2));
    running = false;

    for (auto& t : filozofowie) t.join();
}
```

### Dlaczego to działa?

```
Kolejność blokowania:

Filozof 0: lock(0), lock(1)
Filozof 1: lock(1), lock(2)
Filozof 2: lock(2), lock(3)
Filozof 3: lock(3), lock(4)
Filozof 4: lock(0), lock(4)    ← bierze 0 PRZED 4!

Scenariusz "prawie deadlock":
  Filozof 1: lock(1) ✅, czeka na lock(2)
  Filozof 2: lock(2) ✅, czeka na lock(3)
  Filozof 3: lock(3) ✅, czeka na lock(4)
  Filozof 4: czeka na lock(0)...         ← NIE trzyma widelca 4!
  Filozof 0: lock(0) ✅, czeka na lock(1)

  Filozof 4 NIE trzyma żadnego widelca → NIE MA CYKLU!
  Filozof 3 dostanie widelec 4 → zje → zwolni 3 i 4
  → Filozof 2 dostanie 3 → zje → itd.
  → System się odblokuje ✅
```

### Dowód poprawności

Załóżmy, że istnieje cykl: T₁ → T₂ → ... → Tₖ → T₁, gdzie Tᵢ czeka na zasób trzymany przez Tᵢ₊₁.

- T₁ trzyma zasób r₁ i czeka na r₂ → r₁ < r₂ (bo blokujemy w kolejności rosnącej)
- T₂ trzyma r₂ i czeka na r₃ → r₂ < r₃
- ...
- Tₖ trzyma rₖ i czeka na r₁ → rₖ < r₁

Ale: r₁ < r₂ < r₃ < ... < rₖ < r₁ → **r₁ < r₁** — SPRZECZNOŚĆ! ∎

---

## 4. Rozwiązanie 2: `std::scoped_lock` (C++17)

### Idea

`std::scoped_lock` blokuje **wiele mutexów jednocześnie** bez deadlocka,
używając wewnętrznie algorytmu unikania zakleszczenia.

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <array>
#include <chrono>
#include <atomic>

const int N = 5;
std::array<std::mutex, N> widelce;
std::atomic<bool> running{true};

void filozof(int id) {
    auto& lewy  = widelce[id];
    auto& prawy = widelce[(id + 1) % N];

    while (running) {
        // Myślenie
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        // scoped_lock blokuje oba muteksy ATOMOWO — bez deadlocka
        std::scoped_lock lock(lewy, prawy);

        std::cout << "Filozof " << id << " je!\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(50));

        // Automatyczne zwolnienie przy wyjściu z scope
    }
}
```

### Jak działa `std::scoped_lock` wewnętrznie?

```cpp
// Uproszczona implementacja std::scoped_lock dla 2 mutexów:
template<typename M1, typename M2>
class scoped_lock {
    M1& m1_;
    M2& m2_;
public:
    scoped_lock(M1& m1, M2& m2) : m1_(m1), m2_(m2) {
        std::lock(m1_, m2_);  // ← magia tutaj!
    }
    ~scoped_lock() {
        m1_.unlock();
        m2_.unlock();
    }
};
```

A `std::lock()` używa algorytmu **try-and-back-off**:

```cpp
// Uproszczona implementacja std::lock(m1, m2):
void lock(mutex& m1, mutex& m2) {
    while (true) {
        m1.lock();                    // zablokuj pierwszy
        if (m2.try_lock()) return;    // SPRÓBUJ zablokować drugi
        m1.unlock();                  // nie udało się → zwolnij pierwszy!
                                      // (łamie warunek "hold and wait")
        // Teraz spróbuj w odwrotnej kolejności:
        m2.lock();
        if (m1.try_lock()) return;
        m2.unlock();
    }
}
```

### Wizualizacja: dlaczego nie ma deadlocka

```
Filozof 0: lock(w0), try_lock(w1)
  ├─ try_lock(w1) sukces   → JE ✅
  └─ try_lock(w1) FAIL     → unlock(w0), spróbuj odwrotnie
                              lock(w1), try_lock(w0)
                              ├─ sukces → JE ✅
                              └─ FAIL → unlock(w1), od nowa...

Kluczowa różnica vs naiwna wersja:
  Naiwna:     lock(lewy)  → TRZYMAM  → lock(prawy)  → CZEKAM W NIESKOŃCZONOŚĆ
  scoped_lock: lock(lewy) → try_lock(prawy) → FAIL → ODDAJĘ lewy → próbuję inaczej
                                                       ^^^^^^^^^
                                                       łamie "hold and wait"!
```

---

## 5. Rozwiązanie 3: Kelner (Arbitraż)

### Idea

Dodajemy **kelnera** (arbitra) który kontroluje ile filozofów może jednocześnie
próbować jeść. Jeśli max N-1 filozofów próbuje → deadlock niemożliwy.

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <array>
#include <chrono>
#include <semaphore>   // C++20
#include <atomic>

const int N = 5;
std::array<std::mutex, N> widelce;

// Kelner wpuszcza max N-1 = 4 filozofów do stołu
std::counting_semaphore<N-1> kelner(N-1);
std::atomic<bool> running{true};

void filozof(int id) {
    auto& lewy  = widelce[id];
    auto& prawy = widelce[(id + 1) % N];

    while (running) {
        // Myślenie
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        kelner.acquire();       // "Kelnerze, czy mogę usiąść?"
                                // Max 4 filozofów przy stole jednocześnie

        lewy.lock();            // Teraz bezpiecznie — bierz lewy
        prawy.lock();           // i prawy (deadlock niemożliwy!)

        std::cout << "Filozof " << id << " je!\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(50));

        prawy.unlock();
        lewy.unlock();

        kelner.release();       // "Kelnerze, wstaję od stołu"
    }
}
```

### Dlaczego N-1 filozofów wystarczy?

```
5 filozofów, 5 widelców:

Jeśli 5 filozofów próbuje → każdy bierze lewy → deadlock ☠️

Jeśli max 4 filozofów próbuje:
  4 filozofowie biorą lewy widelec → 4 widelce zajęte, 1 WOLNY
  → Co najmniej 1 filozof MA oba widelce → zje → zwolni → system działa ✅

Ogólnie: przy N-1 filozofach i N widelcach, 
         zawsze jest co najmniej 1 wolny widelec
         → co najmniej 1 filozof może wziąć oba → brak deadlocka
```

### Dowód przez zasadę szufladkową (pigeonhole)

```
N-1 filozofów, N widelców.
Każdy trzyma ≤ 2 widelce.
Każdy trzyma ≥ 1 widelec (wziął lewy).

Widelców zajętych: ≤ 2(N-1) = 2N-2
Ale max N widelców, więc zajętych: ≤ N

Nawet jeśli każdy wziął lewy (N-1 widelców zajętych):
  Wolnych widelców: N - (N-1) = 1
  Ten wolny widelec jest czyimś prawym → ten filozof bierze oba → je → zwalnia ∎
```

---

## 6. Rozwiązanie 4: Chandy/Misra (zaawansowane)

### Idea

Rozwiązanie **rozproszone** — nie wymaga centralnego arbitra.
Każdy widelec ma stan: **czysty** lub **brudny**.

Reguły:
1. Początkowo: widelec należy do filozofa o niższym numerze, jest **brudny**
2. Żeby jeść: wyślij prośbę o brakujące widelce
3. Filozof który trzyma **brudny** widelec i dostanie prośbę → **myje** go i oddaje
4. Filozof który trzyma **czysty** widelec → **nie oddaje** (sam go potrzebuje)
5. Po jedzeniu: widelce stają się **brudne**

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <array>
#include <chrono>
#include <atomic>

const int N = 5;

struct Fork {
    std::mutex mtx;
    int owner;          // kto aktualnie ma widelec
    bool dirty = true;  // brudny = można oddać; czysty = potrzebny
    bool requested = false;  // ktoś poprosił o ten widelec
    std::condition_variable cv;
};

std::array<Fork, N> forks;
std::atomic<bool> running{true};

void init_forks() {
    // Widelec należy do filozofa o niższym numerze
    for (int i = 0; i < N; ++i) {
        int left_phil = i;
        int right_phil = (i + 1) % N;
        forks[i].owner = std::min(left_phil, right_phil);
        forks[i].dirty = true;
    }
}

void request_fork(int phil_id, int fork_id) {
    Fork& f = forks[fork_id];
    std::unique_lock<std::mutex> lock(f.mtx);

    if (f.owner == phil_id) return;  // już mam

    f.requested = true;
    // Czekaj aż właściciel odda (bo widelec jest brudny)
    f.cv.wait(lock, [&] { return f.owner == phil_id; });
}

void release_dirty_forks(int phil_id) {
    // Sprawdź widelce — jeśli brudny i ktoś prosił → oddaj
    for (int fork_id : {phil_id, (phil_id + 1) % N}) {
        Fork& f = forks[fork_id];
        std::lock_guard<std::mutex> lock(f.mtx);
        if (f.owner == phil_id && f.dirty && f.requested) {
            f.dirty = false;     // myję
            f.owner = -1;        // oddaję
            // Ustal nowego właściciela (ten kto prosił)
            int other = (fork_id == phil_id)
                ? (phil_id - 1 + N) % N
                : (phil_id + 1) % N;
            f.owner = other;
            f.requested = false;
            f.cv.notify_all();
        }
    }
}

void filozof(int id) {
    int left_fork  = id;
    int right_fork = (id + 1) % N;

    while (running) {
        // Myślenie
        std::this_thread::sleep_for(std::chrono::milliseconds(50));

        // Oddaj brudne widelce jeśli ktoś prosił
        release_dirty_forks(id);

        // Poproś o widelce których nie mam
        request_fork(id, left_fork);
        request_fork(id, right_fork);

        // Jedzenie
        std::cout << "Filozof " << id << " je!\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(50));

        // Po jedzeniu widelce stają się brudne
        {
            std::lock_guard<std::mutex> l1(forks[left_fork].mtx);
            forks[left_fork].dirty = true;
        }
        {
            std::lock_guard<std::mutex> l2(forks[right_fork].mtx);
            forks[right_fork].dirty = true;
        }

        // Oddaj jeśli ktoś czeka
        release_dirty_forks(id);
    }
}
```

### Wizualizacja stanów widelców

```
Początek: widelec należy do filozofa o niższym ID

  F0─[dirty, owner=0]──F1─[dirty, owner=1]──F2
  │                                           │
  [dirty, owner=0]                   [dirty, owner=2]
  │                                           │
  F4──────[dirty, owner=3]──────────F3

F0 chce jeść:
  Ma widelec 0 ✅ i widelec 4→0 (owner=0) ✅ → JE
  Po jedzeniu: oba brudne

F1 chce jeść:
  Ma widelec 1 ✅
  Prosi o widelec 0 → F0 ma go brudnego → F0 oddaje ✅
  → JE

Klucz: brudny = "już go użyłem, mogę oddać"
       czysty = "dopiero dostałem, jeszcze nie jadłem" → nie oddaję
```

---

## 7. Porównanie rozwiązań

| Rozwiązanie | Deadlock-free | Starvation-free | Max concurrency | Złożoność | Rozproszone? |
|-------------|:---:|:---:|:---:|:---:|:---:|
| Naiwne (lewy→prawy) | ❌ | ❌ | N/A | ⭐ | ✅ |
| Hierarchia zasobów | ✅ | ❌ | 2 z 5 | ⭐⭐ | ✅ |
| `std::scoped_lock` | ✅ | ❌ | 2 z 5 | ⭐ | ✅ |
| Kelner (semafor N-1) | ✅ | ⚠️ Zależy od impl. | 2 z 5 | ⭐⭐ | ❌ (centralny) |
| Chandy/Misra | ✅ | ✅ | 2 z 5 | ⭐⭐⭐⭐ | ✅ |
| Jeden mutex globalny | ✅ | ✅ | 1 z 5 ❌ | ⭐ | ❌ |

---

## 8. Zagłodzenie — analiza

### 8.1 Czy hierarchia zasobów powoduje zagłodzenie?

```
Filozof 4: musi brać widelec 0 PRZED 4
           Ale widelec 0 jest też widelcem Filozofa 0!

Scenariusz zagłodzenia:
  F0 i F1 jedzą na przemian tak szybko, że F4 nigdy
  nie może złapać widelca 0 → F4 GŁODUJE

  F0: lock(0) lock(1) je unlock(1) unlock(0)
  F1:                              lock(1) lock(2) je unlock(2) unlock(1)
  F0: lock(0) lock(1) je unlock(1) unlock(0)
  F1:                              lock(1) lock(2) je ...
  F4: lock(0)? → zajęty... lock(0)? → zajęty... lock(0)? → zajęty... ☠️
```

### 8.2 Demonstracja zagłodzenia

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <array>
#include <atomic>
#include <chrono>

const int N = 5;
std::array<std::mutex, N> widelce;
std::array<std::atomic<int>, N> meal_count{};

void filozof_glodny(int id) {
    int pierwszy = std::min(id, (id + 1) % N);
    int drugi    = std::max(id, (id + 1) % N);

    auto start = std::chrono::steady_clock::now();
    auto limit = start + std::chrono::seconds(5);

    while (std::chrono::steady_clock::now() < limit) {
        // Agresywni filozofowie 0 i 1 — jedzą bez przerwy
        if (id <= 1) {
            // Brak fazy myślenia!
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        widelce[pierwszy].lock();
        widelce[drugi].lock();

        meal_count[id]++;

        widelce[drugi].unlock();
        widelce[pierwszy].unlock();
    }
}

int main() {
    std::array<std::thread, N> filozofowie;
    for (int i = 0; i < N; ++i)
        filozofowie[i] = std::thread(filozof_glodny, i);
    for (auto& t : filozofowie) t.join();

    std::cout << "\nLiczba posiłków:\n";
    for (int i = 0; i < N; ++i)
        std::cout << "  Filozof " << i << ": " << meal_count[i] << "\n";
}
```

**Oczekiwany wynik (przybliżony):**

```
Liczba posiłków:
  Filozof 0: 45000   ← agresywny
  Filozof 1: 43000   ← agresywny
  Filozof 2: 1200
  Filozof 3: 800
  Filozof 4: 150     ← głoduje! (rywalizuje z F0 o widelec 0)
```

---

## 9. Warianty i rozszerzenia problemu

### 9.1 Asymetryczny filozof (jeden leworęczny)

Najprostsze rozwiązanie deadlocka — jeden filozof bierze widelce **w odwrotnej kolejności**:

```cpp
void filozof(int id) {
    auto& lewy  = widelce[id];
    auto& prawy = widelce[(id + 1) % N];

    while (running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        if (id == N - 1) {
            // Ostatni filozof jest "leworęczny" — odwrócona kolejność
            prawy.lock();
            lewy.lock();
        } else {
            lewy.lock();
            prawy.lock();
        }

        std::cout << "Filozof " << id << " je!\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(50));

        // unlock w odwrotnej kolejności do lock
        if (id == N - 1) {
            lewy.unlock();
            prawy.unlock();
        } else {
            prawy.unlock();
            lewy.unlock();
        }
    }
}
```

### Dlaczego to działa?

```
Filozof 0: lock(0), lock(1)
Filozof 1: lock(1), lock(2)
Filozof 2: lock(2), lock(3)
Filozof 3: lock(3), lock(4)
Filozof 4: lock(0), lock(4)    ← bierze 0 (prawy!) przed 4 (lewym!)

To jest dokładnie hierarchia zasobów — Filozof 4 "przypadkiem"
blokuje w kolejności rosnącej (0 < 4).

Cykl niemożliwy z tego samego powodu co w rozwiązaniu 1.
```

### 9.2 try_lock z backoff

Łamie warunek „hold and wait" — filozof **oddaje** widelec gdy nie może wziąć drugiego:

```cpp
void filozof_trylock(int id) {
    auto& lewy  = widelce[id];
    auto& prawy = widelce[(id + 1) % N];

    while (running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        lewy.lock();

        if (prawy.try_lock()) {
            // Mam oba — jem!
            std::cout << "Filozof " << id << " je!\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            prawy.unlock();
            lewy.unlock();
        } else {
            // Nie mogę wziąć prawego — oddaję lewy i spróbuję później
            lewy.unlock();
            // ⚠️ Backoff — czekam losowy czas żeby uniknąć livelocking
            std::this_thread::sleep_for(
                std::chrono::milliseconds(rand() % 10));
        }
    }
}
```

**Uwaga — LIVELOCK!** Bez losowego backoff:

```
Wszyscy: lock(lewy), try_lock(prawy) → FAIL
Wszyscy: unlock(lewy)
Wszyscy: lock(lewy), try_lock(prawy) → FAIL
Wszyscy: unlock(lewy)
... w nieskończoność — nikt nie je, ale nikt nie jest zablokowany!
    To jest LIVELOCK — gorszy niż deadlock (zużywa CPU)
```

### 9.3 Timeout

```cpp
void filozof_timeout(int id) {
    auto& lewy  = widelce[id];
    auto& prawy = widelce[(id + 1) % N];

    while (running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        // std::timed_mutex zamiast std::mutex!
        if (lewy.try_lock_for(std::chrono::milliseconds(100))) {
            if (prawy.try_lock_for(std::chrono::milliseconds(100))) {
                std::cout << "Filozof " << id << " je!\n";
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                prawy.unlock();
                lewy.unlock();
            } else {
                lewy.unlock();  // timeout na prawym → oddaj lewy
            }
        }
        // timeout → spróbuj ponownie w następnej iteracji
    }
}
```

Wymaga zmiany typu widelców:
```cpp
std::array<std::timed_mutex, N> widelce;  // zamiast std::mutex
```

---

## 10. Ćwiczenia praktyczne

### Ćwiczenie 1: Zaobserwuj deadlock 🔍

**Zadanie:**
1. Skompiluj i uruchom naiwną wersję (sekcja 2.1)
2. Uruchom **10 razy** — ile razy program się zakleszczy?
3. Dodaj diagnostykę: wypisuj który filozof wziął który widelec
4. Zmień `sleep_for` między lock(lewy) a lock(prawy) na 0, 1ms, 10ms, 100ms
5. Jak czas sleep wpływa na prawdopodobieństwo deadlocka?

**Tabela wyników:**

| Sleep między lockami | Deadlock w próbie 1? | ... w próbie 5? | ... w próbie 10? |
|:-------------------:|:---:|:---:|:---:|
| 0 ms | | | |
| 1 ms | | | |
| 10 ms | | | |
| 100 ms | | | |

**Pytanie:** Czy brak deadlocka przy 0ms oznacza, że program jest poprawny?

---

### Ćwiczenie 2: Zaimplementuj i porównaj trzy rozwiązania 📊

**Zadanie:** Zaimplementuj system pomiaru wydajności dla trzech rozwiązań:

1. **Hierarchia zasobów** (sekcja 3)
2. **`std::scoped_lock`** (sekcja 4)
3. **Kelner/semafor** (sekcja 5)

Dla każdego zmierz:
- Łączna liczba posiłków w ciągu 5 sekund
- Posiłki per filozof (fairness)
- Max/min posiłków (stosunek max/min = miara nierówności)

**Szkielet pomiaru:**

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <array>
#include <atomic>
#include <chrono>
#include <algorithm>
#include <numeric>
#include <iomanip>

const int N = 5;
std::array<std::mutex, N> widelce;
std::array<std::atomic<int>, N> meals{};
std::atomic<bool> running{true};

// TODO: Zaimplementuj trzy warianty funkcji filozof:
// void filozof_hierarchy(int id);
// void filozof_scoped(int id);
// void filozof_waiter(int id);

void run_benchmark(const std::string& name, void(*filozof_fn)(int)) {
    // Reset
    for (auto& m : meals) m = 0;
    running = true;

    std::array<std::thread, N> threads;
    for (int i = 0; i < N; ++i)
        threads[i] = std::thread(filozof_fn, i);

    std::this_thread::sleep_for(std::chrono::seconds(5));
    running = false;
    for (auto& t : threads) t.join();

    // Statystyki
    int total = 0, min_m = INT_MAX, max_m = 0;
    for (int i = 0; i < N; ++i) {
        int m = meals[i].load();
        total += m;
        min_m = std::min(min_m, m);
        max_m = std::max(max_m, m);
    }

    std::cout << "\n=== " << name << " ===\n";
    for (int i = 0; i < N; ++i)
        std::cout << "  Filozof " << i << ": " << meals[i] << " posiłków\n";
    std::cout << "  Total: " << total << "\n";
    std::cout << "  Fairness (max/min): " << std::fixed << std::setprecision(1)
              << (double)max_m / std::max(min_m, 1) << "x\n";
}

int main() {
    // TODO: Odkomentuj po implementacji:
    // run_benchmark("Hierarchia zasobów", filozof_hierarchy);
    // run_benchmark("std::scoped_lock",   filozof_scoped);
    // run_benchmark("Kelner (semafor)",    filozof_waiter);
}
```

**Tabela wyników do wypełnienia:**

| Rozwiązanie | Total posiłków | Fairness (max/min) | Najgłodniejszy filozof |
|-------------|:-:|:-:|:-:|
| Hierarchia zasobów | | | |
| `std::scoped_lock` | | | |
| Kelner (semafor) | | | |

---

### Ćwiczenie 3: Wykrywanie deadlocka 🕵️

**Zadanie:** Zaimplementuj **detektor deadlocka** jako osobny wątek monitorujący.

Idea: jeśli przez X sekund żaden filozof nie zjadł posiłku → prawdopodobnie deadlock.

```cpp
class DeadlockDetector {
    std::array<std::atomic<int>, N>& meals_;
    std::chrono::seconds timeout_;

public:
    DeadlockDetector(std::array<std::atomic<int>, N>& meals,
                     std::chrono::seconds timeout)
        : meals_(meals), timeout_(timeout) {}

    // Uruchom w osobnym wątku
    void run() {
        // TODO:
        // 1. Co sekundę zapisuj snapshot posiłków
        // 2. Jeśli przez `timeout_` sekund żaden posiłek
        //    nie przybyła → wypisz DEADLOCK DETECTED!
        // 3. Wypisz stan każdego filozofa
        // BONUS: wypisz graf oczekiwania (kto czeka na kogo)
    }
};
```

**Rozszerzenie:** Dodaj logowanie które widelce każdy filozof aktualnie trzyma.
Użyj tego do wypisania **grafu oczekiwania** (wait-for graph) i wykrycia cyklu.

---

### Ćwiczenie 4: Livelock 🏃‍♂️

**Zadanie:**

1. Zaimplementuj wersję z `try_lock` **BEZ** losowego backoff
2. Zaobserwuj livelock — program nie jest zablokowany, ale nikt nie je
3. Dodaj losowy backoff — zmierz jak wartość max backoff wpływa na throughput

```cpp
// TODO: Zaimplementuj
void filozof_livelock(int id) {
    // Wersja BEZ backoff — powinien wystąpić livelock
}

void filozof_backoff(int id, int max_backoff_ms) {
    // Wersja Z losowym backoff — parametryzowalny
}
```

**Tabela wyników:**

| Max backoff | Total posiłków/5s | Livelock? | Fairness |
|:-----------:|:-:|:-:|:-:|
| 0 ms (brak) | | | |
| 1 ms | | | |
| 5 ms | | | |
| 10 ms | | | |
| 50 ms | | | |
| 100 ms | | | |

**Pytania:**
- Jaki max_backoff daje najlepszą przepustowość?
- Czy istnieje trade-off między przepustowością a fairness?
- Czym livelock różni się od deadlocka z perspektywy zużycia CPU?

---

### Ćwiczenie 5: N filozofów 📈

**Zadanie:** Uogólnij rozwiązanie z hierarchią zasobów na **N filozofów**:

1. Zaimplementuj z parametrem N (nie hardcoded 5)
2. Zmierz przepustowość (posiłki/sekundę) dla N = 2, 3, 5, 10, 20, 50, 100
3. Zmierz max concurrency (ilu filozofów jednocześnie je?)

```cpp
// Podpowiedź: max jednocześnie jedzących = floor(N/2)
// N=5  → max 2
// N=10 → max 5
// N=100 → max 50
```

**Tabela:**

| N | Max concurrency (teoria) | Max concurrency (zmierzone) | Posiłki/s |
|:-:|:---:|:---:|:---:|
| 2 | 1 | | |
| 3 | 1 | | |
| 5 | 2 | | |
| 10 | 5 | | |
| 20 | 10 | | |
| 50 | 25 | | |
| 100 | 50 | | |

**Pytanie:** Jak zmienia się przepustowość per filozof gdy N rośnie?
Czy jest granica skalowalności? Dlaczego?

---

### Ćwiczenie 6: Fair Dining Philosophers 🎯

**Zadanie:** Zaimplementuj rozwiązanie **gwarantujące brak zagłodzenia**.

Wymagania:
1. Deadlock-free ✅
2. Starvation-free ✅
3. Żaden filozof nie czeka dłużej niż K rund (gdzie runda = jedno pełne jedzenie każdego filozofa)

**Podpowiedź — ticketing:**

```cpp
class FairDining {
    std::array<std::mutex, N> widelce;
    std::mutex mtx_;
    std::condition_variable cv_;
    std::array<int, N> hunger_level_{};  // ile rund filozof czeka
    // Filozof z najwyższym hunger_level_ ma priorytet

public:
    void eat(int id) {
        // TODO:
        // 1. Zwiększ hunger_level_[id]
        // 2. Czekaj aż:
        //    - Oba widelce wolne
        //    - Nikt z sąsiadów nie ma wyższego hunger_level_
        // 3. Zablokuj widelce, jedz
        // 4. Reset hunger_level_[id] = 0
        // 5. Notify
    }
};
```

**Test: porównaj z hierarchią zasobów na agresywnym scenariuszu:**

```cpp
// Filozofowie 0 i 1 nie myślą (agresywni), reszta myśli 10ms
// Po 5 sekundach porównaj posiłki każdego filozofa
// FairDining powinno dać ~równe wyniki
```

---

### Ćwiczenie 7: Wizualizacja (projekt) 🎨

**Zadanie:** Zaimplementuj wizualizację tekstową ucztujących filozofów w terminalu.

```
┌─────────────────────────────────────┐
│         Ucztujący Filozofowie        │
│              t = 3.7s               │
│                                     │
│            🍴 [F0: THINKING]  🍴    │
│           /                    \    │
│    [F4: EATING]            [F1: WAITING] │
│    🍴🍴                    🍴       │
│          \                  /       │
│        [F3: THINKING]  [F2: EATING] │
│            🍴        🍴🍴          │
│                                     │
│  Posiłki: F0=12 F1=8 F2=14 F3=9 F4=11  │
│  Stan widelców: 0=F4 1=free 2=F2 3=free 4=F4 │
└─────────────────────────────────────┘
```

**Wymagania:**
1. Odświeżaj co 100ms (użyj ANSI escape codes: `\033[H` — kursor na początek)
2. Pokazuj stan każdego filozofa: THINKING / WAITING / EATING
3. Pokazuj kto trzyma który widelec
4. Wyświetlaj licznik posiłków na żywo
5. Podświetl na czerwono gdy wykryjesz potencjalne zagłodzenie (filozof czeka > 2s)

**Szkielet:**

```cpp
#include <iostream>
#include <array>
#include <atomic>
#include <thread>
#include <chrono>

enum class State { THINKING, WAITING, EATING };
const char* state_str[] = {"THINKING", "WAITING ", "EATING  "};
const char* state_emoji[] = {"🤔", "⏳", "🍝"};

std::array<std::atomic<State>, N> states;
std::array<std::atomic<int>, N> fork_owner;  // -1 = free
std::array<std::atomic<int>, N> meals;

void display_thread() {
    while (running) {
        // Wyczyść ekran (ANSI)
        std::cout << "\033[H\033[2J";

        // TODO: Narysuj stan stołu
        // TODO: Wypisz statystyki
        // BONUS: Czerwony kolor dla głodujących

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}
```

---

### Ćwiczenie 8: Analiza warunków Coffmana 📋

**Zadanie teoretyczne** — dla każdego rozwiązania określ, który warunek Coffmana jest złamany:

| Rozwiązanie | Mutual excl. | Hold & wait | No preemption | Circular wait |
|-------------|:---:|:---:|:---:|:---:|
| Naiwne (deadlock!) | ✅ spełniony | ✅ spełniony | ✅ spełniony | ✅ spełniony |
| Hierarchia zasobów | ✅ | ✅ | ✅ | ❓ |
| `std::scoped_lock` | ✅ | ❓ | ✅ | ❓ |
| Kelner (semafor) | ✅ | ❓ | ✅ | ❓ |
| try_lock + backoff | ✅ | ❓ | ❓ | ✅ |
| Jeden globalny mutex | ❓ | ❓ | ✅ | ❓ |
| Chandy/Misra | ✅ | ✅ | ❓ | ❓ |

**Uzupełnij ❓ i uzasadnij odpowiedź.**

<details>
<summary>Podpowiedzi</summary>

- **Hierarchia zasobów** — łamie circular wait (porządek liniowy = brak cyklu)
- **scoped_lock** — łamie hold & wait (bierze oba naraz lub żadnego)
- **Kelner** — łamie circular wait (max N-1 → zawsze 1 wolny widelec → brak cyklu)
- **try_lock + backoff** — łamie hold & wait (oddaje lewy gdy nie może wziąć prawego)
- **Jeden globalny mutex** — łamie hold & wait (nie potrzeba wielu zasobów)
- **Chandy/Misra** — łamie no preemption (brudny widelec jest oddawany na żądanie)

</details>

---

## 11. Mapowanie na problemy rzeczywiste

### Transfer bankowy = 2 filozofów

```cpp
struct Account {
    std::mutex mtx;
    int balance;
};

// ❌ DEADLOCK: transfer(A→B) jednocześnie z transfer(B→A)
void transfer_deadlock(Account& from, Account& to, int amount) {
    std::lock_guard<std::mutex> lock1(from.mtx);
    std::lock_guard<std::mutex> lock2(to.mtx);   // ☠️
    from.balance -= amount;
    to.balance += amount;
}

// ✅ ROZWIĄZANIE: scoped_lock (= ucztujący filozofowie z scoped_lock)
void transfer_safe(Account& from, Account& to, int amount) {
    std::scoped_lock lock(from.mtx, to.mtx);
    from.balance -= amount;
    to.balance += amount;
}

// ✅ ALTERNATYWA: hierarchia (sortuj po adresie/ID konta)
void transfer_hierarchy(Account& from, Account& to, int amount) {
    Account* first  = (&from < &to) ? &from : &to;
    Account* second = (&from < &to) ? &to : &from;
    std::lock_guard<std::mutex> lock1(first->mtx);
    std::lock_guard<std::mutex> lock2(second->mtx);
    from.balance -= amount;
    to.balance += amount;
}
```

---

## 12. Podsumowanie

### Kluczowe wnioski

1. **Deadlock wymaga CZTERECH warunków** — złam jeden i deadlock znika
2. **`std::scoped_lock`** to najłatwiejsze rozwiązanie w C++17 — używaj go domyślnie
3. **Hierarchia zasobów** jest prosta i skuteczna, ale wymaga globalnej numeracji
4. **Livelock** jest podstępniejszy niż deadlock — program zużywa CPU ale nie robi postępu
5. **Zagłodzenie** to osobny problem od deadlocka — brak deadlocka ≠ brak zagłodzenia
6. **Chandy/Misra** gwarantuje brak zagłodzenia, ale jest znacznie bardziej złożone
7. **Problem filozofów = problem wielu locków** — występuje gdy wątek potrzebuje >1 zasobu

### Reguła kciuka

```
Potrzebujesz wielu mutexów jednocześnie?
│
├─ Czy możesz użyć JEDNEGO mutexa?
│  └─ TAK → zrób to (najprostsze rozwiązanie) ✅
│
├─ Musisz mieć wiele mutexów?
│  ├─ Znasz je z góry (compile-time)?
│  │  └─ std::scoped_lock(m1, m2, ...) ✅
│  │
│  ├─ Nie znasz z góry (runtime)?
│  │  └─ Hierarchia zasobów (sortuj po ID/adresie) ✅
│  │
│  └─ Potrzebujesz timeout?
│     └─ try_lock_for() + backoff ✅
│
└─ Potrzebujesz gwarancji fairness?
   └─ Chandy/Misra lub centralna kolejka ✅
```

---

## Literatura

- E.W. Dijkstra, *Hierarchical Ordering of Sequential Processes*, Acta Informatica, 1971



# Warunki Coffmana — precyzyjne sformułowanie i dowód

---

## Uwaga wstępna

Coffman et al. (1971) sformułowali cztery warunki jako **warunki konieczne** deadlocka,
nie jako warunki konieczne **i wystarczające**.

| Twierdzenie | Status |
|-------------|--------|
| Deadlock → wszystkie 4 warunki spełnione | ✅ **Udowodnione** (warunki konieczne) |
| Wszystkie 4 warunki spełnione → deadlock | ❌ **Nieprawda w ogólności!** |

Cztery warunki spełnione jednocześnie oznaczają, że deadlock **może** wystąpić,
ale **nie musi**. Zależy od konkretnego timingu.

---

## Model formalny

### Definicje

- **Procesy** (wątki): T = {t₁, t₂, …, tₙ}
- **Zasoby** (muteksy): R = {r₁, r₂, …, rₘ}, każdy z jedną instancją
- **Funkcja trzymania:** H(tᵢ) ⊆ R — zbiór zasobów trzymanych przez tᵢ
- **Funkcja oczekiwania:** W(tᵢ) ⊆ R — zbiór zasobów na które tᵢ czeka
- **Graf przydziału zasobów** (Resource Allocation Graph, RAG):
  - Wierzchołki: T ∪ R
  - Krawędź żądania: tᵢ → rⱼ gdy rⱼ ∈ W(tᵢ) (wątek czeka na zasób)
  - Krawędź przydziału: rⱼ → tᵢ gdy rⱼ ∈ H(tᵢ) (zasób przydzielony wątkowi)

### Definicja deadlocka

Zbiór procesów D ⊆ T, D ≠ ∅, jest w stanie **deadlocka** wtedy i tylko wtedy gdy:

```
∀ tᵢ ∈ D:  W(tᵢ) ≠ ∅  ∧  ∀ rⱼ ∈ W(tᵢ):  ∃ tₖ ∈ D:  rⱼ ∈ H(tₖ)
```

Słownie: każdy proces w D czeka na zasób trzymany przez inny proces w D.

---

## Cztery warunki — formalne sformułowanie

### Warunek 1: Mutual Exclusion (Wzajemne wykluczanie)

```
∀ rⱼ ∈ R:  |{tᵢ ∈ T : rⱼ ∈ H(tᵢ)}| ≤ 1
```

Każdy zasób może być trzymany przez **co najwyżej jeden** proces w danym momencie.

### Warunek 2: Hold and Wait (Trzymaj i czekaj)

```
∃ tᵢ ∈ T:  H(tᵢ) ≠ ∅  ∧  W(tᵢ) ≠ ∅
```

Istnieje proces, który **trzyma** co najmniej jeden zasób
**i jednocześnie czeka** na kolejny.

### Warunek 3: No Preemption (Brak wywłaszczania)

Zasobu nie można odebrać procesowi siłą — proces sam go zwalnia
dobrowolnie po zakończeniu użycia.

Formalnie: jedyna operacja usuwająca rⱼ z H(tᵢ) to jawne zwolnienie przez tᵢ.

### Warunek 4: Circular Wait (Cykliczne oczekiwanie)

```
∃ t₁, t₂, …, tₖ ∈ T,  k ≥ 2:
  t₁ czeka na zasób trzymany przez t₂
  t₂ czeka na zasób trzymany przez t₃
  ⋮
  tₖ czeka na zasób trzymany przez t₁
```

Formalnie: w **grafie wait-for** istnieje cykl.

**Graf wait-for:** tᵢ → tₖ  wtw.  ∃ rⱼ: rⱼ ∈ W(tᵢ) ∧ rⱼ ∈ H(tₖ)

---

## Twierdzenie (Coffman et al., 1971)

> **Jeśli w systemie wystąpił deadlock, to wszystkie cztery warunki są spełnione.**
>
> (Warunki konieczne)

---

## Dowód (⇒: deadlock implikuje 4 warunki)

Niech D = {t₁, …, tₖ} będzie zbiorem procesów w deadlocku.

### Krok 1: Mutual Exclusion musi zachodzić

Z definicji deadlocka: ∀ tᵢ ∈ D: W(tᵢ) ≠ ∅.

Tzn. każdy tᵢ czeka na jakiś zasób rⱼ. Czekanie oznacza, że tᵢ **nie może uzyskać** rⱼ.
Gdyby zasób mógł być współdzielony (brak mutual exclusion), tᵢ nie musiałby czekać —
mógłby go użyć jednocześnie z obecnym posiadaczem.

Zatem mutual exclusion musi zachodzić (na zasobach zaangażowanych w deadlock). ∎

### Krok 2: Hold and Wait musi zachodzić

Z definicji deadlocka:
- ∀ tᵢ ∈ D: W(tᵢ) ≠ ∅  (każdy czeka)
- Każdy zasób na który czeka tᵢ jest trzymany przez jakiś tₖ ∈ D

Gdyby żaden tᵢ ∈ D nie trzymał żadnego zasobu (tzn. H(tᵢ) = ∅ dla wszystkich tᵢ ∈ D),
to nie byłoby na co czekać — sprzeczność z definicją deadlocka (kto trzyma te zasoby?).

Zatem ∃ tᵢ ∈ D: H(tᵢ) ≠ ∅ ∧ W(tᵢ) ≠ ∅. ∎

### Krok 3: No Preemption musi zachodzić

Gdyby zasoby mogły być wywłaszczone, system mógłby odebrać zasób rⱼ procesowi tₖ
i dać go tᵢ (który na niego czeka). Wtedy tᵢ przestałby czekać → nie byłby w deadlocku.

Zatem: jeśli deadlock trwa, to zasoby **nie mogą być odebrane** siłą. ∎

### Krok 4: Circular Wait musi zachodzić

Konstruujemy cykl:
- t₁ ∈ D czeka na zasób rⱼ₁ ∈ W(t₁)
- rⱼ₁ jest trzymany przez jakiś t₂ ∈ D (z definicji deadlocka)
- t₂ czeka na rⱼ₂, trzymany przez t₃ ∈ D
- Kontynuujemy: t₃ → t₄ → …

Ponieważ D jest **skończony**, w ciągu tym musi pojawić się
powtórzenie: tₛ = tₗ dla s < l.

Zatem mamy cykl: tₛ → tₛ₊₁ → … → tₗ = tₛ. ∎

---

## Dlaczego implikacja w drugą stronę NIE zachodzi

### Kontrprzykład

4 warunki spełnione, ale brak deadlocka:

```
Zasoby: r1, r2 (mutual exclusion ✅)
Procesy: t1, t2

Stan:
  t1: trzyma r1, MOŻE poprosić o r2 (ale jeszcze nie poprosił)
  t2: trzyma r2, MOŻE poprosić o r1 (ale jeszcze nie poprosił)

Analiza warunków:
  1. Mutual exclusion ✅ (r1 i r2 exclusive)
  2. Hold and wait    ✅ (oba trzymają i MOGĄ czekać)
  3. No preemption    ✅ (nie zabieramy siłą)
  4. Circular wait    ✅ (POTENCJALNY cykl t1→t2→t1)
```

```
Linia czasu — scenariusz BEZ deadlocka:

t1: [trzyma r1, pracuje......., unlock(r1), lock(r2), pracuje, unlock(r2)]
t2: [trzyma r2, pracuje, lock(r1)──czeka──, ──dostaje r1!──, pracuje.....]
                                                ↑
                                    t1 zwolnił r1 zanim poprosił o r2
                                    → brak cyklu → brak deadlocka

Linia czasu — scenariusz Z deadlockiem:

t1: [trzyma r1, lock(r2)��─czeka──────── ☠️]
t2: [trzyma r2, lock(r1)──czeka──────── ☠️]
                    ↑
        Obaj poprosili zanim zwolnili → cykl → deadlock
```

Warunki opisują **strukturę systemu** umożliwiającą deadlock, ale **czy deadlock
faktycznie wystąpi** zależy od konkretnego **przeplotu** (interleaving) operacji.

---

## Kiedy warunki SĄ wystarczające?

Dla **zasobów z jedną instancją** (tak jak w problemie filozofów —
każdy widelec jest unikalny):

> **Twierdzenie:** W systemie z zasobami jednoinstancyjnymi, deadlock istnieje
> **wtedy i tylko wtedy** gdy graf wait-for zawiera cykl.

### Dowód (⇐: cykl w grafie wait-for → deadlock)

Niech cykl to: t₁ → t₂ → … → tₖ → t₁.

- t₁ → t₂ oznacza: t₁ czeka na zasób trzymany przez t₂
- t₂ nie może zwolnić tego zasobu, bo sam czeka (na zasób trzymany przez t₃)
- t₃ nie może zwolnić, bo czeka na t₄
- …
- tₖ nie może zwolnić, bo czeka na t₁

Żaden proces w cyklu nie może uczynić postępu → deadlock. ∎

**Uwaga:** Dla zasobów z wieloma instancjami (np. semafor z count > 1)
cykl w RAG jest konieczny ale **nie wystarczający** — potrzeba dodatkowej
analizy (algorytm bankiera Dijkstry).

---

## Podsumowanie — co dokładnie wiemy

```
┌─────────────────────────────────────────────────────────────┐
│                                                             │
│   ┌─────────────────────────────────────────────────────┐   │
│   │ 4 warunki Coffmana spełnione                        │   │
│   │                                                     │   │
│   │  ┌──────────────────────────────┐                   │   │
│   │  │ DEADLOCK                     │                   │   │
│   │  │ (wystarczy złamać            │ ← deadlock jest   │   │
│   │  │  jeden warunek               │   TUTAJ           │   │
│   │  │  aby wyeliminować)           │   (podzbiór)      │   │
│   │  └──────────────────────────────┘                   │   │
│   │                                                     │   │
│   │  Reszta tego zbioru = systemy ZAGROŻONE             │   │
│   │  deadlockiem ale akurat timing uratował             │   │
│   └─────────────────────────────────────────────────────┘   │
│                                                             │
│   Poza zbiorem = systemy BEZPIECZNE                         │
│   (co najmniej 1 warunek Coffmana złamany)                  │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

### Praktyczna implikacja

Mimo że 4 warunki nie gwarantują deadlocka, **łamanie warunków jest
skuteczną strategią prewencji**, bo:

| Strategia | Co robimy | Pewność |
|-----------|-----------|---------|
| **Prewencja** (złam warunek) | Uniemożliwiamy deadlock strukturalnie | 100% — deadlock **niemożliwy** |
| **Unikanie** (algorytm bankiera) | Dynamicznie odmawiamy przydziału | 100% — wymaga wiedzy z góry |
| **Detekcja + recovery** | Pozwalamy na deadlock, wykrywamy, naprawiamy | Deadlock chwilowo możliwy |
| **Ignorowanie** (struś) | Liczymy na szczęście | 🙃 |

> Złamanie **jednego** warunku Coffmana → warunki konieczne niespełnione
> → deadlock **logicznie niemożliwy**. Dlatego ta strategia działa
> pomimo tego, że warunki nie są wystarczające.

---

## Literatura

- E.G. Coffman, M.J. Elphick, A. Shoshani, *System Deadlocks*, Computing Surveys 3(2), 1971
- A. Silberschatz, P. Galvin, G. Gagne, *Operating System Concepts*, rozdziały 7–8
- A.S. Tanenbaum, *Modern Operating Systems*, rozdział 6
