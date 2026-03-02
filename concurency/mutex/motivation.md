
## Wzajemne wykluczanie bez mechanizmów synchronizacyjnych

Struktura każdego wątku:

```cpp
void process() {
    while (true) {
        own_stuff();         // własne sprawy
        entry_protocol();    // protokół wstępny  <-- co tu wpisać?
        critical_section();  // sekcja krytyczna
        exit_protocol();     // protokół końcowy   <-- co tu wpisać?
    }
}
```

### Próba 1 — zmienna `who_waits` (strict alternation)

Pomysł: zmienna `who_waits` mówi, kto musi czekać. Po wyjściu z sekcji krytycznej przekazujemy pierwszeństwo drugiemu.

```cpp
#include <iostream>
#include <thread>
#include <atomic>

std::atomic<int> who_waits{1};  // kto musi czekać (0 lub 1)

void process(int id) {
    while (true) {
        // own_stuff();
        while (who_waits == id) { /* busy wait */ }  // protokół wstępny
        // --- sekcja krytyczna ---
        who_waits = id;                               // protokół końcowy
    }
}

int main() {
    std::thread t1(process, 0);
    std::thread t2(process, 1);
    t1.join();
    t2.join();
    return 0;
}
```

a może `whose_turn`?
```cpp
std::atomic<int> whoose_turn{0};  // czyja kolej (0 lub 1)

void process(int id) {
    int other = 1 - id;
    while (true) {
        // own_stuff();
        while (whoose_turn == other) { /* busy wait */ }  // czekaj, dopóki kolej rywala
        // --- sekcja krytyczna ---
        whoose_turn = other;                               // oddaj kolej rywalowi
    }
}
```
**Analiza:**

- ✅ **Bezpieczeństwo: TAK** — nigdy oba w sekcji krytycznej.
- ❌ **Żywotność: NIE** — procesy są ściśle powiązane. Jeśli P₁ nie chce wchodzić do sekcji krytycznej (zostaje w `own_stuff()`), to P₂ nie może wejść, bo czeka na swoją kolej. **Ścisła alternacja.**

### Próba 2 — zmienne `is_in` (sprawdź, potem ustaw)

Pomysł: każdy proces ma flagę mówiącą, czy jest w sekcji krytycznej. Przed wejściem sprawdzamy flagę rywala, a potem ustawiamy swoją.

```cpp
std::atomic<bool> is_in[2];  // is_in[0], is_in[1]; init false

void process(int id) {
    int other = 1 - id;
    while (true) {
        // own_stuff();
        while (is_in[other]) { /* busy wait */ }  // (A) sprawdź rywala
        is_in[id] = true;                         // (B) ustaw swoją flagę
        // --- sekcja krytyczna ---
        is_in[id] = false;
    }
}
```

**Analiza:**

- ✅ **Żywotność: TAK** — procesy nie są ze sobą ściśle powiązane. Jeśli jeden nie chce wchodzić, drugi wchodzi swobodnie.
- ❌ **Bezpieczeństwo: NIE** — między sprawdzeniem flagi rywala (A) a ustawieniem swojej flagi (B) może nastąpić przełączenie kontekstu:

| Krok | P₁                                           | P₂                                           | is_in₁ | is_in₂ |
|------|-----------------------------------------------|-----------------------------------------------|--------|--------|
| 1    | `while(is_in2)` → false, wychodzi z pętli    |                                               | F      | F      |
| 2    |                                               | `while(is_in1)` → false, wychodzi z pętli    | F      | F      |
| 3    | `is_in1 = true`                               |                                               | T      | F      |
| 4    |                                               | `is_in2 = true`                               | T      | T      |
| 5    | **Oba w sekcji krytycznej!**                  | **Oba w sekcji krytycznej!**                  | T      | T      |

### Próba 3 — zmienne `wants` (ustaw, potem sprawdź)

Wniosek z próby 2: ustawiamy flagę za późno. Odwróćmy kolejność — najpierw deklarujemy chęć, potem sprawdzamy rywala.

```cpp
std::atomic<bool> wants[2];  // wants[0], wants[1]; init false

void process(int id) {
    int other = 1 - id;
    while (true) {
        // own_stuff();
        wants[id] = true;                         // (A) deklaruj chęć
        while (wants[other]) { /* busy wait */ }  // (B) sprawdź rywala
        // --- sekcja krytyczna ---
        wants[id] = false;
    }
}
```

**Analiza:**

- ✅ **Bezpieczeństwo: TAK** — jeśli oba ustawiły flagę, to oba utkną w pętli `while`.
- ❌ **Żywotność: NIE** — właśnie dlatego! Jeśli oba ustawią flagi przed sprawdzeniem, **oba czekają w nieskończoność** — **deadlock (zakleszczenie)**:

| Krok | P₁                               | P₂                               | wants₁ | wants₂ |
|------|-----------------------------------|-----------------------------------|--------|--------|
| 1    | `wants1 = true`                  |                                   | T      | F      |
| 2    |                                   | `wants2 = true`                  | T      | T      |
| 3    | `while(wants2)` → kręci się…     |                                   | T      | T      |
| 4    |                                   | `while(wants1)` → kręci się…     | T      | T      |
|      | **→ Deadlock! Żaden nie wejdzie do sekcji krytycznej.** | | | |

### Próba 4 — chwilowe ustępowanie

Ratujemy próbę 3: w pętli chwilowo wycofujemy chęć, żeby dać szansę rywalowi.

```cpp
std::atomic<bool> wants[2];  // wants[0], wants[1]; init false

void process(int id) {
    int other = 1 - id;
    while (true) {
        // own_stuff();
        wants[id] = true;
        while (wants[other]) {
            wants[id] = false;   // ustąp
            wants[id] = true;    // spróbuj ponownie
        }
        // --- sekcja krytyczna ---
        wants[id] = false;
    }
}
```

**Analiza:**

- ✅ **Bezpieczeństwo: TAK** — wejście do sekcji krytycznej wymaga, by flaga rywala była `false`.
- ❌ **Żywotność: NIE** — istnieje (złośliwy) przeplot, w którym oba procesy synchronicznie ustawiają i wycofują flagi w nieskończoność — **livelock**:

| Krok | P₁                               | P₂                               | wants₁ | wants₂ |
|------|-----------------------------------|-----------------------------------|--------|--------|
| 1    | `wants1 = true`                  | `wants2 = true`                  | T      | T      |
| 2    | `while(wants2)` → T              | `while(wants1)` → T              | T      | T      |
| 3    | `wants1 = false`                 | `wants2 = false`                 | F      | F      |
| 4    | `wants1 = true`                  | `wants2 = true`                  | T      | T      |
| 5    | `while(wants2)` → T              | `while(wants1)` → T              | T      | T      |
|      | ⋮ **powtarza się w nieskończoność — livelock!** | | | |

**Uwaga:** Argument „w praktyce ten przeplot jest mało prawdopodobny" **nie jest** akceptowalny. Skoro *istnieje* scenariusz naruszający żywotność, program jest **niepoprawny**.

### Podsumowanie prób

| Próba                            | Bezp. | Żywot. | Problem            |
|----------------------------------|-------|--------|--------------------|
| 1. `who_waits` (alternacja)     | ✅ TAK | ❌ NIE  | Ścisła alternacja  |
| 2. `is_in` (sprawdź, ustaw)     | ❌ NIE | ✅ TAK  | Race condition     |
| 3. `wants` (ustaw, sprawdź)     | ✅ TAK | ❌ NIE  | Deadlock           |
| 4. `wants` + ustępowanie        | ✅ TAK | ❌ NIE  | Livelock           |
| **Peterson**                     | ✅ **TAK** | ✅ **TAK** | **Poprawny!** |

---

## Algorytm Petersena — poprawne rozwiązanie

Sprytne połączenie prób 1 i 3: utrzymujemy flagi `wants`, a konflikty rozstrzygamy zmienną `who_waits`.

```cpp
#include <iostream>
#include <thread>
#include <atomic>

std::atomic<bool> wants1{false};
std::atomic<bool> wants2{false};
std::atomic<int>  who_waits{1};

void process1() {
    while (true) {
        // own_stuff();

        wants1 = true;          // deklaruję chęć wejścia
        who_waits = 1;          // ustępuję: "ja mogę poczekać"
        while (wants2 && who_waits == 1) {
            /* busy wait */
        }

        // --- sekcja krytyczna ---

        wants1 = false;         // opuszcza sekcję krytyczną
    }
}

void process2() {
    while (true) {
        // own_stuff();

        wants2 = true;
        who_waits = 2;
        while (wants1 && who_waits == 2) {
            /* busy wait */
        }

        // --- sekcja krytyczna ---

        wants2 = false;
    }
}

int main() {
    std::thread t1(process1);
    std::thread t2(process2);

    t1.join();
    t2.join();

    return 0;
}
```

### Dlaczego to działa?

**Bezpieczeństwo:** Załóżmy, że oba wątki są w sekcji krytycznej. Wtedy `wants1 = true` i `wants2 = true`. Aby P₁ przeszedł przez pętlę, musiałby zajść warunek `who_waits ≠ 1`, czyli `who_waits = 2`. Analogicznie P₂ potrzebuje `who_waits = 1`. Ale `who_waits` nie może być jednocześnie 1 i 2 — **sprzeczność**. ∎

**Żywotność:** Załóżmy, że P₁ utknął w pętli `while`. Zatem `wants2 = true` i `who_waits = 1`. Rozważmy dwa przypadki:

1. P₂ nie chce wejść do sekcji ⟹ `wants2 = false` ⟹ P₁ wychodzi z pętli. Sprzeczność.
2. P₂ chce wejść ⟹ wykona `who_waits = 2` ⟹ P₁ wychodzi z pętli (bo `who_waits ≠ 1`). Sprzeczność. ∎

### Kluczowe pytania do analizy

| Pytanie | Odpowiedź |
|---------|-----------|
| Czy można zamienić kolejność `wants = true` i `who_waits = i`? | ❌ **NIE** |
| Czy można przenieść `who_waits = i` do protokołu końcowego? | ❌ **NIE** |
| Czy można zamienić kolejność sprawdzania warunków w `while`? | ✅ TAK |
| Czy można zmienić wartość początkową `who_waits`? | ✅ TAK (1 lub 2) |
| Czy można zmienić wartość początkową `wants`? | ❌ **NIE** (musi być `false`) |

### Wady algorytmu Petersena

1. **Aktywne oczekiwanie (busy waiting / spinlock)** — wątek w pętli `while` zużywa czas procesora, nie robiąc nic pożytecznego. Potrzebne wsparcie OS, aby wątek mógł zostać uśpiony (np. `std::mutex`, semafory).
2. **Ograniczenie do dwóch wątków** — rozszerzenie na *n* wątków wymaga *n − 1* „barier".

---

## Algorytm Petersena dla *n* wątków (uogólnienie)

Uogólnienie polega na zastąpieniu jednej bariery ciągiem *n − 1* barier. Każda bariera zatrzymuje jeden proces — zatem *n − 1* barier przepuści co najwyżej jeden.

```cpp
#include <iostream>
#include <thread>
#include <atomic>
#include <vector>

const int N = 5;  // liczba wątków

std::atomic<int> level[N];       // level[i] = do której bariery doszedł wątek i
std::atomic<int> who_waits[N];   // who_waits[b] = kto czeka na barierze b

// Inicjalizacja: level[i] = 0 dla każdego i

void process(int id) {
    while (true) {
        // own_stuff();

        // Protokół wstępny: przejdź przez N-1 barier
        for (int barrier = 1; barrier < N; ++barrier) {
            level[id] = barrier;
            who_waits[barrier] = id;

            // Czekaj, dopóki istnieje rywal na tym samym lub wyższym poziomie
            // i to ja jestem tym, który ma czekać
            bool rival_ahead = true;
            while (rival_ahead && who_waits[barrier] == id) {
                rival_ahead = false;
                for (int j = 0; j < N; ++j) {
                    if (j != id && level[j] >= barrier) {
                        rival_ahead = true;
                        break;
                    }
                }
            }
        }

        // --- sekcja krytyczna ---

        // Protokół końcowy
        level[id] = 0;
    }
}

int main() {
    for (int i = 0; i < N; ++i) {
        level[i] = 0;
    }

    std::vector<std::thread> threads;
    for (int i = 0; i < N; ++i) {
        threads.emplace_back(process, i);
    }
    for (auto& t : threads) {
        t.join();
    }

    return 0;
}
```

### Wady wersji *n*-wątkowej

1. **Aktywne oczekiwanie** — jak w wersji dla 2 wątków.
2. **Znana z góry liczba wątków** — *n* musi być stałą.
3. **Duża złożoność protokołu wstępnego** — O(n²) w najgorszym przypadku (*n − 1* barier × sprawdzenie *n − 1* rywali).

> **Wniosek:** Algorytm Petersena dowodzi, że wzajemne wykluczanie jest *możliwe* bez specjalnych mechanizmów synchronizacyjnych.
>
> Jednak w praktyce używamy `std::mutex`, semaforów i monitorów, które unikają aktywnego oczekiwania dzięki wsparciu systemu operacyjnego.
