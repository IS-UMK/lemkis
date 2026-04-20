# Pula zadań przygotowawczych — Programowanie współbieżne w C++

## Zasady

- Zadania są przeznaczone do rozwiązywania **na kartce** — nie wymagają kompilatora.
- Nacisk na **zrozumienie mechanizmów**, nie na składnię C++.
- Pseudokod jest akceptowalny tam, gdzie zaznaczono.

---

# 🟢 KATEGORIA 1: ŁATWE

---

### Zadanie 1.1 — Deadlock w Thread Pool

Mamy Thread Pool z **2 wątkami**. Rozważ poniższy kod:

```cpp
ThreadPool pool(2);

auto f1 = pool.submit([&pool] {
    auto inner1 = pool.submit([] { return 10; });
    return inner1.get() + 5;
});

auto f2 = pool.submit([&pool] {
    auto inner2 = pool.submit([] { return 20; });
    return inner2.get() + 5;
});

std::cout << f1.get() << " " << f2.get() << std::endl;
```

**(a)** Wykaż, że ten kod prowadzi do deadlocka. Opisz krok po kroku, co robią Worker-1 i Worker-2 oraz jaki jest stan kolejki.

**(b)** Gdybyśmy usunęli `f2` (zostawili tylko `f1`), czy deadlock nadal jest możliwy? Uzasadnij, analizując ile wątków jest wolnych w momencie, gdy `inner1` trafia do kolejki.

**(c)** Zaproponuj **dwa różne** sposoby rozwiązania problemu z oryginalnego kodu (bez zwiększania puli do nieskończoności).

---

### Zadanie 1.2 — Livelock vs Deadlock

Rozważ filozofów z podejściem `try_lock` **bez losowego backoff**:

```cpp
void filozof(int id) {
    while (true) {
        lewy.lock();
        if (prawy.try_lock()) {
            // je
            prawy.unlock();
            lewy.unlock();
        } else {
            lewy.unlock();
            // natychmiast próbuje ponownie
        }
    }
}
```

**(a)** Opisz, dlaczego ten kod może prowadzić do **livelocking** — podaj konkretny scenariusz z 5 filozofami.

**(b)** Czym livelock różni się od deadlocka z perspektywy: (i) zużycia CPU, (ii) możliwości wykrycia, (iii) zachowania programu?

**(c)** Dlaczego **losowy** backoff pomaga, a **stały** (np. zawsze 5 ms) nie musi?

---

### Zadanie 1.3 — Dangling reference w lambdzie

Rozważ:

```cpp
void schedule_work(ThreadPool& pool) {
    std::vector<int> data = {1, 2, 3, 4, 5};

    pool.submit([&data] {
        int sum = 0;
        for (int x : data) sum += x;
        std::cout << sum << "\n";
    });
}
```

**(a)** Wyjaśnij, dlaczego ten kod zawiera błąd, mimo że wygląda poprawnie.

**(b)** Napraw ten kod. Podaj **dwa różne** poprawne rozwiązania i opisz trade-off między nimi.

**(c)** Czy zamiana `[&data]` na `[&]` rozwiązuje problem? Uzasadnij.

---

### Zadanie 1.4 — Zagłodzenie w readers-preference

Mamy implementację czytelników-pisarzy z preferencją czytelników:

```cpp
void read_lock() {
    std::unique_lock lock(mtx_);
    cv_.wait(lock, [this] { return !writer_active_; });
    ++active_readers_;
}
```

**(a)** Podaj dokładny scenariusz (sekwencję zdarzeń), w którym pisarz **nigdy** nie uzyska dostępu, mimo że ciągle próbuje.

**(b)** Jak zmodyfikować predykat w `read_lock()`, żeby dać pisarzom priorytet? Wystarczy jedna linia — napisz ją i wyjaśnij, dlaczego działa.

**(c)** Czy Twoje rozwiązanie z (b) może teraz zagłodzić **czytelników**? Jeśli tak — opisz scenariusz.

---

### Zadanie 1.5 — Data race ukryta za `atomic`

Rozważ:

```cpp
ThreadPool pool(4);
int result = 0;

for (int i = 0; i < 1000; ++i) {
    pool.submit([&result, i] {
        result += i;
    });
}
// czekamy na zakończenie...
std::cout << result << "\n";
```

**(a)** Wyjaśnij, dlaczego wynik jest **niedeterministyczny**, mimo że wydaje się, iż po prostu sumujemy liczby.

**(b)** Ktoś proponuje naprawę: `std::atomic<int> result{0};` z `result += i;`. Czy to wystarczy do poprawności? Uzasadnij.

**(c)** Zaproponuj rozwiązanie **bez** `std::atomic`, które jest poprawne i potencjalnie wydajniejsze (podpowiedź: użyj `future`).

---

### Zadanie 1.6 — Upgrade locka

Rozważ funkcję, która czyta wartość i warunkowo ją modyfikuje:

```cpp
void maybe_reset(std::shared_mutex& rw, int& data) {
    std::shared_lock<std::shared_mutex> slock(rw);
    if (data < 0) {
        std::unique_lock<std::shared_mutex> ulock(rw);
        data = 0;
    }
}
```

**(a)** Wyjaśnij, dlaczego ten kod prowadzi do **deadlocka**.

**(b)** Napisz poprawną wersję tej funkcji. Wyjaśnij, dlaczego po uzyskaniu `unique_lock` musisz **ponownie sprawdzić** warunek `data < 0`.

**(c)** Jak nazywa się ten wzorzec (sprawdź → zwolnij → zablokuj exclusive → sprawdź ponownie → działaj)? Podaj analogię z innego obszaru programowania.

---

### Zadanie 1.7 — `packaged_task` i `shared_ptr`

W implementacji `submit()` zwracającego `future` używamy:

```cpp
auto task = std::make_shared<std::packaged_task<R()>>(
    std::bind(std::forward<F>(f), std::forward<Args>(args)...)
);
tasks_.push([task]() { (*task)(); });
```

**(a)** Dlaczego `packaged_task` jest opakowany w `shared_ptr` zamiast być umieszczony bezpośrednio w lambdzie?

**(b)** Dlaczego `get_future()` musi być wywołane **przed** wrzuceniem task do kolejki?

**(c)** Co się stanie z wyjątkiem rzuconym wewnątrz `packaged_task`? Porównaj z wersją `submit(std::function<void()>)` bez `future`.

---

### Zadanie 1.8 — Backpressure

Mamy Thread Pool z **nieograniczoną** kolejką (`std::queue`). Producent generuje zadania 100× szybciej niż konsumenci je przetwarzają.

**(a)** Co się stanie z pamięcią programu po dłuższym czasie działania?

**(b)** Jak `BoundedThreadPool` (z ograniczoną kolejką) rozwiązuje ten problem? Co dokładnie dzieje się z wątkiem producenta, gdy kolejka jest pełna?

**(c)** Podaj realny system (z życia), w którym brak backpressure doprowadził(by) do katastrofy i wyjaśnij mechanizm.

---

### Zadanie 1.9 — Filozof z timeoutem

Rozważ rozwiązanie problemu filozofów z `try_lock_for`:

```cpp
void filozof(int id) {
    while (true) {
        if (lewy.try_lock_for(100ms)) {
            if (prawy.try_lock_for(100ms)) {
                // je
                prawy.unlock();
            }
            lewy.unlock();
        }
    }
}
```

**(a)** Który warunek Coffmana jest tu złamany? Uzasadnij.

**(b)** Czy to rozwiązanie gwarantuje brak **zagłodzenia**? Podaj scenariusz, w którym filozof mimo wszystko nie je przez długi czas.

**(c)** Czy zwiększenie timeoutu (np. z 100 ms na 10 s) poprawia czy pogarsza sytuację? Rozważ wpływ na throughput i fairness.

---

### Zadanie 1.10 — Poison pill vs flaga shutdown

Porównaj dwa podejścia do graceful shutdown w systemie producent-konsument z **3 producentami** i **5 konsumentami**:

**Podejście A — flaga `shutdown_`:**
```
close() → shutdown_ = true → notify_all()
```

**Podejście B — poison pill:**
```
producent na końcu wstawia specjalną wartość -1
```

**(a)** W podejściu B, ile wartości „-1" musi wstawić każdy producent? Dlaczego? Co się stanie, jeśli wstawi za mało?

**(b)** W podejściu B, konsument po odebraniu -1 robi `bufor.push(-1)` i kończy pracę. Wyjaśnij, dlaczego to „przekazanie dalej" jest konieczne.

**(c)** Podaj typ `T` danych w buforze, dla którego podejście B jest **niemożliwe** do zaimplementowania w naturalny sposób. Dlaczego podejście A nie ma tego ograniczenia?

---

# 🟡 KATEGORIA 2: ŚREDNIE

---

### Zadanie 2.1 — Deadlock bez cyklu w RAG

Na wykładzie udowodniono, że dla zasobów **jednoinstancyjnych** deadlock ⟺ cykl w grafie wait-for. Ale dla zasobów **wieloinstancyjnych** cykl jest konieczny, lecz **niewystarczający**.

Podaj konkretny przykład systemu z:
- co najmniej 3 procesami,
- co najmniej 2 typami zasobów (z wieloma instancjami),
- **cyklem** w grafie RAG (Resource Allocation Graph),
- ale **brakiem deadlocka**.

Narysuj graf RAG i wyjaśnij, dlaczego deadlock nie zachodzi mimo cyklu.

---

### Zadanie 2.2 — Fair Dining Philosophers

Zaproponuj rozwiązanie problemu ucztujących filozofów, które jest:
1. **Deadlock-free**
2. **Starvation-free** (żaden filozof nie głoduje)
3. Nie używa centralnego arbitra (kelnera/semafora)
4. Nie używa hierarchii zasobów

Opisz (pseudokodem lub słownie) algorytm filozofa i udowodnij (lub przekonująco uzasadnij), że spełnia warunki 1–2.

---

### Zadanie 2.3 — Thread Pool z rekurencyjnymi zadaniami

Mamy Thread Pool z `N = 4` wątkami. Zadanie `parallel_merge_sort` rekurencyjnie dzieli tablicę i zleca podzadania na **ten sam** pool.

**(a)** Rozważ schemat, w którym **obie** połówki zlecamy przez `pool.submit()`:

```
parallel_merge_sort(pool, data):
    if data.size() <= 1: return
    left, right = split(data)
    future_left  = pool.submit(parallel_merge_sort, pool, left)
    future_right = pool.submit(parallel_merge_sort, pool, right)
    future_left.get()
    future_right.get()
    merge(left, right, data)
```

Wykaż, że ten schemat prowadzi do deadlocka. Przy jakiej minimalnej głębokości rekursji wszystkie 4 wątki zostaną zablokowane?

**(b)** Teraz rozważ poprawiony schemat, w którym **jedną** połówkę sortujemy w bieżącym wątku:

```
parallel_merge_sort(pool, data):
    if data.size() <= 1: return
    left, right = split(data)
    future_left = pool.submit(parallel_merge_sort, pool, left)
    parallel_merge_sort(pool, right)   // ← w BIEŻĄCYM wątku!
    future_left.get()
    merge(left, right, data)
```

Wykaż, że ten schemat **nie prowadzi** do deadlocka, niezależnie od głębokości rekursji. Ile **jednoczesnych** oczekujących `future.get()` może być w systemie?

**(c)** Dlaczego w schemacie z (b) nie da się zablokować więcej wątków niż wynosi rozmiar puli, mimo że zadań rekurencyjnych może być tysiące?

---

### Zadanie 2.4 — RCU a lost update

Rozważ RCU-like cache z C++20 (`std::atomic<std::shared_ptr<...>>`):

```cpp
class RCUConfig {
    std::atomic<std::shared_ptr<const Map>> config_;

public:
    shared_ptr<const Map> read() const {
        return config_.load();
    }

    void update(string key, string value) {
        auto old = config_.load();
        shared_ptr<Map> new_config;
        do {
            new_config = make_shared<Map>(*old);
            (*new_config)[key] = value;
        } while (!config_.compare_exchange_weak(old, new_config));
    }
};
```

**(a)** Dwaj pisarze jednocześnie wywołują `update("x", "A")` i `update("x", "B")`. Opisz sekwencję zdarzeń, w której **jeden z zapisów jest utracony** (lost update). Czy to jest bug w implementacji?

**(b)** Trzej pisarze jednocześnie wywołują `update` z różnymi kluczami. W najgorszym przypadku, ile razy zostanie **niepotrzebnie skopiowana** mapa? Wyjaśnij, dlaczego pętla `do-while` z `compare_exchange_weak` jest konieczna.

**(c)** Zaproponuj sposób rozwiązania problemu z (a), zakładając, że chcemy, aby oba zapisy zostały zachowane (tzn. mapa powinna zawierać zarówno zmianę A jak i B, o ile dotyczą różnych kluczy). Jaki jest trade-off Twojego rozwiązania?

---

### Zadanie 2.5 — Filozofowie z asymetrią obciążenia

Pięciu filozofów siedzi przy stole, ale:
- Filozof 0 i 1 to „naukowcy" — myślą 100 ms, jedzą 1 ms
- Filozof 2, 3, 4 to „łakomczuchy" — myślą 1 ms, jedzą 100 ms

Używamy rozwiązania z hierarchią zasobów (każdy bierze widelec o niższym numerze najpierw).

**(a)** Który filozof jest najbardziej poszkodowany w tym scenariuszu? Uzasadnij, analizując rywalizację o poszczególne widelce.

**(b)** Zaproponuj **bez zmiany algorytmu blokowania** (nadal hierarchia zasobów) sposób na poprawienie fairness. Podpowiedź: czy zmiana **numeracji** widelców/filozofów może pomóc?

**(c)** Czy istnieje przypisanie numerów widelców do pozycji przy stole, które minimalizuje nierówność w tym konkretnym scenariuszu? Uzasadnij lub podaj kontrargument.

---

### Zadanie 2.6 — Pipeline z różnymi prędkościami etapów

Mamy pipeline 3-etapowy na jednym Thread Pool:

```
Generator (1ms/elem) → Transformer (10ms/elem) → Writer (1ms/elem)
```

Każdy etap to osobne zadanie na poolu. Pomiędzy etapami są `BoundedBuffer` o rozmiarze `B`.

**(a)** Jaki jest **bottleneck** tego pipeline'u? Jaka jest teoretyczna przepustowość (elementów/sekundę)?

**(b)** Ktoś proponuje rozwiązanie: „użyjmy 10 wątków Transformera zamiast jednego". Opisz, jak zmodyfikować architekturę (pseudokod lub schemat), aby uruchomić 10 równoległych instancji etapu 2. Jakie nowe problemy mogą się pojawić? (Podpowiedź: kolejność elementów.)

**(c)** Jaki rozmiar `B` (bufora między etapami) jest **optymalny**? Rozważ skrajne przypadki: `B = 1` i `B = 10000`. Jakie są trade-offy?

---

### Zadanie 2.7 — Dowód poprawności hierarchii zasobów

Udowodnij formalnie (metodą „nie wprost"), że hierarchia zasobów (blokowanie mutexów w kolejności rosnącej ich numerów) uniemożliwia deadlock.

Wskazówki:
1. Załóż, że deadlock istnieje — czyli istnieje cykl w grafie wait-for: T₁ → T₂ → ... → Tₖ → T₁.
2. Tᵢ → Tᵢ₊₁ oznacza: Tᵢ trzyma zasób rᵢ i czeka na zasób rᵢ₊₁ trzymany przez Tᵢ₊₁.
3. Z zasady hierarchii: Tᵢ trzyma rᵢ i czeka na rᵢ₊₁, a skoro blokuje w kolejności rosnącej...
4. Wyprowadź sprzeczność.

---

### Zadanie 2.8 — Wykrywanie deadlocka

Zaproponuj algorytm **detektora deadlocka** dla systemu z `N` wątkami i `M` mutexami. Detektor działa jako osobny wątek monitorujący.

**(a)** Jakie informacje musi zbierać detektor? Zaproponuj struktury danych.

**(b)** Napisz pseudokod algorytmu wykrywania cyklu w grafie wait-for.

**(c)** Wyjaśnij, dlaczego detektor sam może wprowadzić problemy wydajnościowe. Jakie informacje musi odczytywać, i jak to zrobić **thread-safe** bez istotnego wpływu na wydajność monitorowanych wątków?

---

### Zadanie 2.9 — Bounded buffer bez condition variable

Zaproponuj implementację `BoundedBuffer` (producent-konsument z ograniczonym buforem), która **nie używa `condition_variable`**, a jedynie:
- `std::mutex`
- `std::atomic`
- operacje `sleep_for`

**(a)** Napisz pseudokod `push()` i `pop()`.

**(b)** Wyjaśnij, dlaczego Twoje rozwiązanie jest **poprawne** (brak data race'ów, producent blokuje się gdy bufor pełny, konsument gdy pusty).

**(c)** Jakie są **wady** tego podejścia w porównaniu z wersją z `condition_variable`? Podaj co najmniej trzy konkretne problemy i wyjaśnij, jak `condition_variable` je rozwiązuje.

---

### Zadanie 2.10 — Wzajemna zależność pool-ów

Mamy **dwa** Thread Poole: Pool A (3 wątki) i Pool B (3 wątki). Zadania na Pool A mogą zlecać podzadania na Pool B i czekać na wynik (`future.get()`), i vice versa.

**(a)** Podaj minimalny scenariusz (ile zadań na każdym poolu), w którym dochodzi do **cross-pool deadlocka**.

**(b)** Czy zwiększenie liczby wątków w obu poolach do dowolnie dużej liczby `N` **gwarantuje** brak deadlocka? Uzasadnij.

**(c)** Zaproponuj architekturę/zasadę, która **strukturalnie** uniemożliwia cross-pool deadlock bez ograniczania funkcjonalności (tj. zadania nadal mogą zlecać podzadania na inny pool). Podpowiedź: pomyśl o grafie zależności między pool-ami.

---

# 🔴 KATEGORIA 3: TRUDNE

Modyfikacje klasycznych problemów, które nie pojawiły się na wykładzie. Wymagają twórczego połączenia poznanych mechanizmów i wymyślenia nowych rozwiązań.

---

### Zadanie 3.1 — Filozofowie z widelcami o różnej wadze

Pięciu filozofów siedzi przy stole, ale widelce nie są jednakowe. Każdy widelec `i` ma **wagę** `w(i)`. Filozof, żeby zjeść, potrzebuje dwóch sąsiednich widelców o **łącznej wadze ≥ 10**. Jeśli suma wag jest < 10, filozof nie może jeść tą parą i musi poczekać, aż wagi się zmienią.

Haczyk: po każdym posiłku waga użytych widelców **maleje o 1** (widelce się zużywają), a osobny wątek „zmywarki" co jakiś czas przywraca wagę wybranego widelca do wartości początkowej (wymaga wyłącznego dostępu do widelca).

**(a)** Wyjaśnij, dlaczego klasyczne rozwiązanie z hierarchią zasobów **nie wystarczy** — filozof może zablokować dwa widelce, których suma wag jest za mała. Jaki nowy problem pojawia się poza deadlockiem?

**(b)** Zaproponuj protokół filozofa, który:
- jest deadlock-free,
- nie trzyma widelca, którego nie może użyć (nie blokuje niepotrzebnie widelca o niskiej wadze),
- pozwala zmywarce bezpiecznie modyfikować wagę widelca.

Opisz pseudokodem cykl życia filozofa i zmywarki. Jakie mechanizmy synchronizacji stosujesz i dlaczego?

**(c)** Czy Twoje rozwiązanie jest starvation-free? Jeśli nie — opisz scenariusz zagłodzenia i zaproponuj poprawkę.

---

### Zadanie 3.2 — Producent-konsument z priorytetami i anulowaniem

Rozważ zmodyfikowany problem producent-konsument, w którym:
- Każde zadanie ma **priorytet** (integer, wyższy = ważniejszy).
- Producent może **anulować** zadanie, które już wrzucił do bufora, ale które nie zostało jeszcze pobrane przez konsumenta (zadanie identyfikowane po unikalnym ID).
- Konsument zawsze pobiera zadanie o **najwyższym priorytecie**.

**(a)** Zaproponuj strukturę danych wewnątrz bufora i pseudokod operacji: `submit(task, priority) → id`, `cancel(id) → bool`, `pop() → task`. Jakie problemy synchronizacyjne pojawiają się przy `cancel()`?

**(b)** Rozważ scenariusz: producent robi `cancel(id)` dokładnie w momencie, gdy konsument wykonuje `pop()` i wyciąga właśnie to zadanie. Opisz, jakie warunki wyścigu mogą wystąpić i jak im zapobiec.

**(c)** Dodajemy ograniczenie: bufor ma rozmiar `B`. Producent blokuje się, gdy bufor jest pełny. Ale `cancel()` zwalnia miejsce! Opisz, jak poprawnie obudzić zablokowanego producenta po anulowaniu zadania. Jakie pułapki tu czyhają? (Podpowiedź: co jeśli między `cancel()` a obudzeniem producenta ktoś inny zdąży zapełnić bufor?)

---

### Zadanie 3.3 — Czytelnicy-pisarze z wieloma zasobami

Mamy system z **trzema niezależnymi zasobami** R1, R2, R3 (np. trzy tabele w bazie danych). Każdy zasób chroniony jest osobnym `shared_mutex`. Przychodzą transakcje, z których każda wymaga dostępu do **podzbioru** zasobów:

- Typ A: czyta R1, czyta R2
- Typ B: pisze R1, czyta R3
- Typ C: czyta R2, pisze R3
- Typ D: pisze R1, pisze R2

**(a)** Pokaż, że naiwne blokowanie zasobów w kolejności wymienionej w typie transakcji (np. typ B: `write_lock(R1)`, potem `read_lock(R3)`) może prowadzić do **deadlocka**. Podaj konkretny scenariusz z dwoma współbieżnymi transakcjami.

**(b)** Zaproponuj protokół blokowania, który **gwarantuje** brak deadlocka dla wszystkich typów transakcji. Udowodnij poprawność.

**(c)** Twoim rozwiązaniem z (b) jest prawdopodobnie hierarchia (blokuj R1 przed R2 przed R3). Ale to oznacza, że transakcja typu C (`czyta R2, pisze R3`) musi najpierw zablokować R2, potem R3. W tym czasie R2 jest zablokowany shared, a R3 jeszcze nie. Czy inny pisarz może „wcisnąć się" i zmodyfikować R3 między tymi dwoma lockami? Jakie to ma konsekwencje dla **spójności** odczytanych danych i jak temu zaradzić?

---

### Zadanie 3.4 — Filozofowie ze zmienną liczbą widelców do jedzenia

Modyfikacja problemu filozofów: filozof `i` potrzebuje **`k(i)` sąsiednich widelców** żeby jeść, gdzie `k(i)` może być różne dla różnych filozofów (ale zawsze ≥ 2 i ≤ N−1).

Przykład dla N = 7 filozofów:
- Filozof 0: potrzebuje widelców {0, 1} (k=2)
- Filozof 1: potrzebuje widelców {1, 2, 3} (k=3) — trzy kolejne widelce!
- Filozof 2: potrzebuje widelców {2, 3} (k=2)
- ...itd.

**(a)** Czy rozwiązanie z hierarchią zasobów (bierz w kolejności rosnących numerów) nadal działa poprawnie? Uzasadnij formalnie.

**(b)** Czy rozwiązanie z kelnerem (semafor N−1) nadal gwarantuje brak deadlocka? Jeśli nie — jaka powinna być wartość semafora? Udowodnij lub podaj kontrprzykład.

**(c)** Rozważ skrajny przypadek: N = 5 filozofów, Filozof 0 potrzebuje widelców {0, 1, 2, 3} (k=4). Jaka jest **maksymalna liczba filozofów**, którzy mogą jednocześnie jeść? Czy `std::scoped_lock` z 4 argumentami rozwiązuje problem deadlocka?

---

### Zadanie 3.5 — Thread Pool z kradzieżą zadań (work stealing) — analiza

Mamy Thread Pool z 4 wątkami. Zamiast jednej wspólnej kolejki, każdy worker ma **swoją lokalną kolejkę**. Gdy lokalna kolejka jest pusta, worker „kradnie" zadanie z kolejki innego workera.

**(a)** W klasycznym thread pool z jedną kolejką mamy jeden `mutex` chroniący kolejkę. W wersji z work stealing mamy 4 kolejki. Intuicyjnie powinno być mniej contention — ale kradzież wymaga blokowania cudzej kolejki. Opisz dokładnie, jakie operacje wymagają jakiego locka (na czyjej kolejce), i kiedy contention nadal występuje.

**(b)** Worker kradnie z **końca** cudzej kolejki (deque — double-ended queue), a swoje zadania bierze z **początku**. Wyjaśnij, dlaczego ta asymetria (LIFO lokalnie, FIFO kradzież) jest korzystna z perspektywy **cache locality**.

**(c)** Rozważ scenariusz: zadanie A jest zlecane, tworzy podzadanie B, które tworzy podzadanie C. Wszystko na tym samym workerze (lokalnie). Porównaj zachowanie **jednej wspólnej kolejki** vs **lokalna kolejka + work stealing** pod kątem: (i) cache locality, (ii) latencji łańcucha A→B→C, (iii) sprawiedliwości wobec innych workerów. Czy work stealing ma jakąś wadę w tym scenariuszu?

---

### Zadanie 3.6 — Producent-konsument z ekspirującymi elementami

Rozważ zmodyfikowany bounded buffer, w którym każdy element ma **czas życia (TTL)**. Element wstawiony do bufora, który nie zostanie skonsumowany w ciągu `T` milisekund, **wygasa** i powinien zostać usunięty (nie dostarczony do konsumenta).

**(a)** Zaproponuj modyfikację `BoundedBuffer`, która obsługuje TTL. Pseudokod `push(item, ttl)` i `pop()`. Kluczowe pytanie: kto i kiedy usuwa wygasłe elementy? Rozważ co najmniej dwa podejścia (leniwe vs aktywne) i porównaj ich wady.

**(b)** Wygaśnięcie elementu powinno **zwolnić miejsce** w buforze, umożliwiając zablokowanemu producentowi kontynuację. Opisz mechanizm powiadamiania producenta o zwolnieniu miejsca przez ekspirację. Jakie pułapki synchronizacyjne tu występują?

**(c)** Konsument wywołuje `pop()` i dostaje element, który wygasł 1 μs temu (sprawdzenie TTL i pobranie nie jest atomowe z upływem czasu). Czy to jest problem? Zaproponuj semantykę: czy `pop()` powinien zwracać wygasłe elementy, czy je pomijać? Jakie są konsekwencje obu podejść dla poprawności i wydajności?

---

### Zadanie 3.7 — Czytelnicy-pisarze z operacją „upgrade" bez deadlocka

Na wykładzie powiedziano, że upgrade `shared_lock → unique_lock` prowadzi do deadlocka. Twoim zadaniem jest zaprojektowanie mechanizmu `upgradeable_lock`, który pozwala na bezpieczny upgrade.

Zasady:
- W danym momencie może istnieć **co najwyżej jeden** `upgradeable_lock` (oprócz dowolnej liczby zwykłych `shared_lock`).
- Holder `upgradeable_lock` może czytać (jak `shared_lock`).
- Holder `upgradeable_lock` może w dowolnym momencie zrobić **upgrade** do trybu exclusive, czekając aż wszyscy inni czytelnicy skończą.
- Zwykli czytelnicy (`shared_lock`) **nie mogą** robić upgrade.

**(a)** Jakie stany musi rozróżniać Twój mutex? Podaj tabelę stanów (analogiczną do tabeli z wykładu dla `shared_mutex`): kto może wejść, a kto czeka, w zależności od bieżącego stanu.

**(b)** Napisz pseudokod: `read_lock()`, `read_unlock()`, `upgrade_lock()`, `upgrade_to_exclusive()`, `upgrade_unlock()`, `write_lock()`, `write_unlock()`. Jakie zmienne stanu (counters/flagi) potrzebujesz?

**(c)** Wykaż, że Twoje rozwiązanie jest deadlock-free. Dlaczego ograniczenie „co najwyżej jeden upgradeable_lock" jest **konieczne**? Pokaż scenariusz deadlocka przy dwóch jednoczesnych `upgrade_to_exclusive()`.

---

### Zadanie 3.8 — Filozofowie-chirurdzy: zasoby współdzielone i wyłączne jednocześnie

Pięciu chirurgów operuje przy okrągłym stole. Między każdą parą leży **narzędzie**. Narzędzia są dwojakiego rodzaju:
- **Skalpel** (narzędzia 0, 2, 4) — może być używany przez **jednego** chirurga naraz (exclusive, jak widelec).
- **Lampa** (narzędzia 1, 3) — może być współdzielona przez **dwóch** sąsiednich chirurgów jednocześnie (shared), ALE wymaga exclusive, gdy chirurg chce ją **przestawić** (co robi co 5-ty posiłek).

Chirurg `i` potrzebuje narzędzia `i` i `(i+1) % 5`.

**(a)** Opisz, jakie typy locków (shared/exclusive) chirurg `i` musi wziąć na swoich narzędziach w zależności od tego, jakie narzędzia ma po lewej i prawej stronie, i czy potrzebuje przestawić lampę.

**(b)** Zaproponuj rozwiązanie deadlock-free. Czy hierarchia zasobów nadal działa, gdy dwa typy locków (shared i exclusive) przeplatają się na tych samych zasobach? Rozważ konkretny scenariusz.

**(c)** Chirurg 1 ma po lewej lampę (narzędzie 1, shared), po prawej skalpel (narzędzie 2, exclusive). Chirurg 2 ma po lewej skalpel (narzędzie 2, exclusive), po prawej lampę (narzędzie 3, shared). Obaj próbują operować jednocześnie. Opisz dokładną sekwencję blokowania z hierarchią zasobów i pokaż, że nie ma deadlocka. Następnie pokaż, co się dzieje, gdy chirurg 1 potrzebuje **przestawić** lampę 1 (exclusive na narzędziu 1).

---

### Zadanie 3.9 — Thread Pool z zależnościami między zadaniami (DAG scheduler)

Rozważ Thread Pool, w którym zadania mają **zależności**: zadanie B może być wykonane dopiero po zakończeniu zadania A. Zależności tworzą **DAG** (skierowany graf acykliczny).

```
Przykład:
    A ──→ C ──→ E
    B ──→ C
    B ──→ D ──→ E

A i B mogą startować natychmiast (brak zależności).
C czeka na A i B.
D czeka na B.
E czeka na C i D.
```

**(a)** Zaproponuj interfejs i pseudokod takiego schedulera. Jakie struktury danych potrzebujesz, żeby śledzić, które zadania mogą być już uruchomione? Jak atomowo zmniejszać licznik niespełnionych zależności?

**(b)** Opisz pułapkę: co się stanie, jeśli zadanie A rzuci **wyjątek**? Zadanie C zależy od A — czy powinno zostać uruchomione? Zaproponuj semantykę propagacji błędów w DAG-u zależności.

**(c)** Wykaż, że jeśli graf zależności jest faktycznie **DAG** (acykliczny), to Twój scheduler jest **deadlock-free**, niezależnie od liczby wątków w puli (nawet jeśli jest 1 wątek). Następnie pokaż, że jeśli użytkownik **pomyłkowo** doda cykliczną zależność (A→B→A), to Twój scheduler powinien to wykryć — jak?

---

### Zadanie 3.10 — Czytelnicy-pisarze z wersjonowaniem (MVCC)

Zamiast blokować czytelników podczas zapisu, rozważ podejście **Multi-Version Concurrency Control** (MVCC): pisarz tworzy **nową wersję** danych, a czytelnicy zawsze widzą spójną (starą) wersję.

**(a)** Zaproponuj implementację (pseudokod) struktury danych, która:
- Pozwala wielu czytelnikom czytać **bez żadnego locka** (nawet shared)
- Pisarz tworzy nową wersję danych i atomowo „publikuje" ją
- Stare wersje są usuwane, gdy żaden czytelnik ich nie używa

Jakie mechanizmy C++ (z wykładu) wykorzystujesz? Podpowiedź: `shared_ptr`.

**(b)** Problem odzyskiwania pamięci (garbage collection wersji): czytelnik może trzymać `shared_ptr` do starej wersji arbitralnie długo. Co się stanie z zużyciem pamięci, jeśli pisarz modyfikuje dane 1000 razy na sekundę, a pewien czytelnik trzyma referencję do wersji sprzed godziny?

**(c)** Porównaj to podejście z klasycznym `shared_mutex` pod kątem:
- Throughput czytelników (który jest lepszy i dlaczego?)
- Koszt zapisu (który jest droższy i dlaczego?)
- Spójność (czy czytelnik widzi „stale" dane? Czy to problem?)
- Zużycie pamięci

Kiedy MVCC jest lepszy, a kiedy gorszy od `shared_mutex`?

---

### Zadanie 3.11 — Graceful shutdown z gwarancją postępu

Mamy system producent-konsument z `BoundedBuffer` i operacją `close()`. Po wywołaniu `close()`:
- Producenci nie mogą wstawiać nowych elementów.
- Konsumenci powinni **dokończyć** przetwarzanie elementów **już będących** w buforze.

Ale pojawia się dodatkowe wymaganie: każdy element przetwarzany przez konsumenta **może generować nowe elementy** (np. crawler — przetwarzanie strony generuje linki do nowych stron). Te nowe elementy **też** powinny być przetworzone przed zakończeniem.

**(a)** Wyjaśnij, dlaczego prosta flaga `shutdown_` z wykładu **nie wystarczy** — po `close()` konsumenci mogą generować nowe elementy, które muszą trafić do bufora. Ale `push()` odrzuca nowe elementy gdy `shutdown_ == true`.

**(b)** Zaproponuj protokół „drain shutdown", który:
1. Blokuje **zewnętrznych** producentów (żadne nowe zadania z zewnątrz).
2. Pozwala **konsumentom** wstawiać nowe elementy do bufora (wewnętrzna generacja).
3. Kończy pracę, gdy bufor jest pusty **i** żaden konsument nie przetwarza elementu (bo mógłby wygenerować nowe).

Pseudokod operacji `drain_close()`, zmodyfikowanego `push()` i `pop()`. Jakie nowe zmienne stanu potrzebujesz?

**(c)** Wykaż, że Twój protokół **terminuje** (kiedyś się kończy) pod warunkiem, że łańcuch generacji jest skończony (każdy element generuje skończoną liczbę nowych elementów, a głębokość łańcucha jest ograniczona). Co się stanie, jeśli ten warunek **nie** jest spełniony?

---

### Zadanie 3.12 — Problem „śpiących fryzjerów" jako wariant producent-konsument

W salonie fryzjerskim jest **K foteli** i **poczekalnia** z **N krzesłami**. Fryzjerzy (wątki workerów) śpią, gdy nie ma klientów. Klienci (producenci) przychodzą losowo:
- Jeśli jest wolny fotel — klient siada i budzi fryzjera.
- Jeśli nie ma wolnego fotela, ale jest wolne krzesło w poczekalni — klient czeka.
- Jeśli poczekalnia jest pełna — klient **odchodzi** (zadanie odrzucone).

**(a)** Zamodeluj ten problem jako wariant producent-konsument z Thread Poolem. Jakie elementy odpowiadają: buforowi, producentom, konsumentom? Czym różni się to od klasycznego `BoundedThreadPool`? (Podpowiedź: w thread pool zadanie czeka w kolejce; tutaj mamy **dwa poziomy** buforowania — fotele i poczekalnię.)

**(b)** Napisz pseudokod synchronizacji. Klient musi atomowo sprawdzić: (1) czy jest wolny fotel, jeśli nie → (2) czy jest wolne krzesło, jeśli nie → (3) odejść. Fryzjer musi atomowo: (1) sprawdzić fotel, (2) jeśli klient jest → strzyc, (3) jeśli nie → sprawdzić poczekalnię, (4) jeśli ktoś czeka → przenieść na fotel, (5) jeśli nikt → zasnąć.

**(c)** Rozważ wariant: fryzjerzy mają **specjalizacje** (np. fryzjer 1 strzyże tylko mężczyzn, fryzjer 2 tylko kobiety, fryzjer 3 — obu). Klient musi trafić do **odpowiedniego** fryzjera. Jak to modyfikuje synchronizację? Czy może dojść do zagłodzenia klienta, który czeka na specjalistę, mimo że inni fryzjerzy są wolni?

---
