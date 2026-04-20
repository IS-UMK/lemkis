# Compare-and-Swap (CAS) w C++ — Wykład i Laboratorium

Czas trwania: ~100 minut (50 min wykład + 50 min laboratoria)

---

## CZĘŚĆ 1: WYKŁAD (50 min)

### 1. Definicja: Czym jest Compare-and-Swap?
**Compare-and-Swap (CAS)** to elementarna, sprzętowo wspierana instrukcja atomowa procesora. 
Działa w następujący sposób: atomowo (niepodzielnie) porównuje aktualną zawartość pewnej komórki pamięci z wartością *oczekiwaną*. Jeśli są one równe, komórka pamięci jest aktualizowana nową wartością. Jeśli nie – operacja się nie powodzi (i najczęściej zwraca aktualną wartość).

### 2. Wyjaśnienie intuicyjne: O co chodzi i po co to jest?
**Wyobraź sobie edycję dokumentu w chmurze (np. Wiki/Google Docs):**
1. Pobierasz na swój komputer wersję dokumentu (wersja nr 1).
2. Spędzasz 10 minut na wprowadzaniu poprawek.
3. Chcesz zapisać dokument. System robi w tym momencie CAS: sprawdza, czy na serwerze dokument to nadal "wersja nr 1". 
4. Jeśli tak – system zapisuje twoją nową wersję.
5. Jeśli nie (bo ktoś inny w międzyczasie zapisał swoją "wersję nr 2") – system **odrzuca** twój zapis, żeby nie nadpisać pracy innej osoby (tzw. zjawisko *Lost Update*).
6. Co robisz? Pobierasz nową wersję nr 2, nanosisz swoje poprawki jeszcze raz i znów próbujesz zapisać.

**Po co nam to w programowaniu?**
Standardowo do ochrony danych przed wyścigami (Data Races) używamy `std::mutex`. Mutex usypia inne wątki, co jest kosztowne (wymaga interwencji systemu operacyjnego, tzw. *context switch*). CAS pozwala na programowanie **Lock-free** – wątki nie zasypiają, tylko aktywnie (w pętli) próbują zaktualizować zmienną. Jest to znacznie szybsze przy krótkich operacjach.

### 3. Jak to działa w C++?
W C++ mechanizm CAS realizowany jest przez metody na obiektach `std::atomic<T>`:
*   `compare_exchange_weak(T& expected, T desired)`
*   `compare_exchange_strong(T& expected, T desired)`

**Co się dzieje podczas wywołania `atomic_var.compare_exchange_weak(expected, desired)`?**
1. Jeśli `atomic_var == expected`, to `atomic_var` przyjmuje wartość `desired` i funkcja zwraca `true`.
2. Jeśli `atomic_var != expected`, to funkcja zwraca `false`, a wartość `expected` jest **automatycznie nadpisywana** tym, co faktycznie znajduje się w `atomic_var` (czyli "pobieramy nową wersję dokumentu z serwera").

**Weak vs Strong:**
Wersja `weak` może czasem zwrócić `false`, nawet jeśli wartości są równe (tzw. *spurious failure* wynikający z budowy cache'u niektórych procesorów, np. ARM). Ponieważ i tak używamy CAS w pętli, wersja `weak` jest często wydajniejsza. Wersja `strong` nigdy nie ma *spurious failures*, ale może być sprzętowo wolniejsza.

### 4. Główny Wzorzec: Pętla CAS (CAS Loop)
To najważniejszy fragment kodu, z którego będziecie korzystać. Służy do atomowej aktualizacji zmiennej na podstawie jej poprzedniego stanu.

```cpp
std::atomic<int> data{0};

void bezpieczna_aktualizacja() {
    int old_val = data.load(); // 1. Pobierz aktualną wersję
    int new_val;
    do {
        new_val = old_val + 5; // 2. Wylicz nową wartość na podstawie starej
    } while (!data.compare_exchange_weak(old_val, new_val)); // 3. Próbuj zapisać
}
```

---

## CZĘŚĆ 2: LABORATORIUM (50 min)

### Zadanie 1 (Rozgrzewka): Atomowy Max (Poziom: Łatwy)
Biblioteka standardowa posiada `fetch_add`, ale nie posiada `fetch_max`.
Napisz funkcję, która z użyciem wzorca CAS aktualizuje `std::atomic<int> max_val` tak, aby zawsze zawierał on maksimum z dotychczasowej wartości i nowej wartości proponowanej przez wątek.

```cpp
void atomic_update_max(std::atomic<int>& max_val, int val_to_check) {
    // TODO: Uzupełnij implementację
}
```

<details>
<summary><b>Kliknij, aby zobaczyć rozwiązanie i formalny dowód poprawności</b></summary>

**Rozwiązanie:**
```cpp
void atomic_update_max(std::atomic<int>& max_val, int val_to_check) {
    int old_val = max_val.load();
    do {
        // Optymalizacja: jeśli val_to_check nie jest większe, nie musimy nic robić
        if (val_to_check <= old_val) {
            break; 
        }
    } while (!max_val.compare_exchange_weak(old_val, val_to_check));
}
```

**Dowód poprawności:**
*Cel:* Udowodnić, że ostateczna wartość w `max_val` to największa wartość ze wszystkich przesłanych `val_to_check` ze wszystkich wątków.
1. **Brak utraconych aktualizacji (No lost updates):** Modyfikacja zmiennej następuje tylko wewnątrz instrukcji CAS, która jest sprzętowo linearyzowalna (podzielna w czasie na dyskretne punkty). Pętla kończy się sukcesem tylko wtedy, gdy punkt odczytu `old_val` (przez CAS po faulcie lub load na początku) i zapis `val_to_check` nie przedzieliła inna modyfikacja.
2. **Monotoniczność:** Jeśli wątek T1 udaje się wykonać CAS (przypisuje `val_to_check`), to z warunku `val_to_check > old_val` wynika, że nowa wartość `max_val` jest ściśle większa od poprzedniej.
3. Wątek T2 proponujący wartość $V_{mniejszą}$ wykona krok `if (val_to_check <= old_val) break;` i nie zmodyfikuje stanu globalnego. Wątek proponujący globalne maksimum ostatecznie zawsze znajdzie stan `old_val < val_to_check` i (pomijając teoretyczne zagłodzenie) wykona swój zapis. Zmienna zbiega zawsze do globalnego maksimum.
</details>

---

### Zadanie 2: Bezpieczny Atomowy Mnożnik (Poziom: Łatwy)
Chcemy atomowo pomnożyć zmienną przez stałą wartość, np. chcemy, by wiele wątków mogło wywołać `multiply(data, 2)`.

```cpp
void atomic_multiply(std::atomic<int>& data, int factor) {
    // TODO: Zaimplementuj przy użyciu CAS
}
```

<details>
<summary><b>Kliknij, aby zobaczyć rozwiązanie i formalny dowód poprawności</b></summary>

**Rozwiązanie:**
```cpp
void atomic_multiply(std::atomic<int>& data, int factor) {
    int old_val = data.load();
    int new_val;
    do {
        new_val = old_val * factor;
    } while (!data.compare_exchange_weak(old_val, new_val));
}
```

**Dowód poprawności:**
1. Zmienna `data` ulega zmianie z wartości stanu $S_i$ na stan $S_{i+1}$ wyłącznie przez atomową instrukcję CAS.
2. Skuteczny CAS gwarantuje, że $S_{i+1} = S_i \times factor$ (ponieważ `old_val` odzwierciedla prawdziwy stan w momencie tuż przed zapisem).
3. Ponieważ mnożenie liczb całkowitych jest przemienne ($a \times b \times c = a \times c \times b$), kolejność, w jakiej poszczególne wątki uaktualniają zmienną globalną (linearyzacja zdarzeń CAS), nie wpływa na ostateczny wynik liczbowy. Ostateczny stan to zawsze $S_0 \times \prod f_j$.
</details>

---

### Zadanie 3: Problem ABA (Poziom: Średni)
Wyobraź sobie implementację stosu bez blokad (Lock-free stack) za pomocą listy jednokierunkowej. Wskaźnik na początek stosu to `std::atomic<Node*> head`.
* Wątek 1 chce zdjąć (`pop`) element A z wierzchołka, który wskazuje na B (`A -> B`). Oczekuje, że `head` == A i chce zamienić `head` na B.
* Zanim Wątek 1 wykona swój CAS, zostaje wywłaszczony (usypia).
* Wątek 2 wchodzi, robi `pop` (zdejmuje A), potem znów `pop` (zdejmuje B, niszczy B w pamięci). Następnie dodaje zupełnie nowy element C, a potem dodaje stary element A (odzyskany z puli pamięci). Stan to teraz `A -> C`.
* Wątek 1 się budzi, wykonuje CAS. Sprawdza: czy `head == A`? Tak! Więc zamienia `head` na B (ponieważ pamięta stan z przeszłości). 

**Pytanie:** Wytłumacz teoretycznie, dlaczego struktura danych ulegnie w tym momencie korupcji. Z czego dokładnie wynika problem z perspektywy CAS?

```cpp
struct Node {
    int data;
    Node* next;
};

std::atomic<Node*> head;

Node* pop() {
    // 1. Odczytujemy aktualną głowę stosu (A)
    Node* old_head = head.load(); 

    while (old_head != nullptr) {
        // 2. Zapisujemy sobie na "kartce" (w zmiennej lokalnej), 
        // co ma być NOWĄ głową, jeśli CAS się powiedzie (B).
        // To jest właśnie ten krytyczny moment!
        Node* next_node = old_head->next; 

        // ==========================================
        // 💥 PUNKT WYWŁASZCZENIA: Wątek 1 zasypia tutaj
        // ==========================================

        // 3. Próbujemy atomowo podmienić head.
        // Tłumaczenie na polski: 
        // "Jeśli head to nadal old_head, zmień go na next_node".
        if (head.compare_exchange_weak(old_head, next_node)) {
            return old_head; // Sukces! Zdjęliśmy element.
        }
    }
    return nullptr; // Stos był pusty
}
```


<details>
<summary><b>Kliknij, aby zobaczyć rozwiązanie i formalny dowód uszkodzenia (Korupcji)</b></summary>

**Rozwiązanie i formalne uzasadnienie:**
Ten błąd nazywa się klasycznym **Problemem ABA**. 
1. Istotą działania instrukcji CAS w C/C++ na wskaźnikach jest to, że sprawdza ona jedynie **równość wartości bitowych adresu pamięci**, a nie tożsamość logiczną historii stanów. Z punktu widzenia CAS, "stare A" oraz "nowe A wstawione ponownie" to te same bity.
2. **Dowód Uszkodzenia (Dangling Pointer / Memory Leak):**
   * Stan zapamiętany przez Wątek 1 przed uśpieniem: `old_head = A`, a pożądany `new_head = A->next = B`.
   * Prawdziwy stan w pamięci w momencie przebudzenia: `head -> A -> C`. (Element B został zniszczony/zwolniony z pamięci).
   * Wątek 1 wykonuje: `CAS(head, expected=A, desired=B)`. Instrukcja porównuje wartość `head` z adresem `A`. Adresy są równe, więc CAS zwraca sukces i zmienia wartość `head` na stary zapisany adres `B`.
   * **Skutek 1:** Od tej pory nowa "głowa" stosu wskazuje na `B`, ale `B` to zwolniony obszar pamięci (Use-After-Free). Dostęp do głowy zakończy się w najgorszym razie błedem typu Segmentation Fault.
   * **Skutek 2:** Element `C`, który był wpięty pod `A`, nie posiada już wskaźnika prowadzącego do niego z `head`. Staje się osieroconym kawałkiem pamięci (Memory Leak).

*Sposób naprawy:* Stosowanie wskaźników z licznikiem generacji (tzw. Tagged Pointers / Double-word CAS) lub mechanizmów takich jak Hazard Pointers (C++26) / RCU.
</details>

---

### Zadanie 4: Dodawanie do struktury Lock-free (Poziom: Trudny)
Zaimplementuj w pełni lock-free metodę `push` dla stosu opartego o listę jednokierunkową. Zwróć uwagę, że operacja `push` nie jest podatna na problem ABA, co ułatwia sprawę.

Struktura węzła:
```cpp
struct Node {
    int data;
    Node* next;
    Node(int d) : data(d), next(nullptr) {}
};

std::atomic<Node*> head{nullptr};
```

```cpp
void lock_free_push(int value) {
    // TODO: 
    // 1. Utwórz nowy Node.
    // 2. Zapnij jego next na aktualny head.
    // 3. Atomowo podmień head na nowy element z użyciem pętli CAS.
}
```

<details>
<summary><b>Kliknij, aby zobaczyć rozwiązanie i formalny dowód</b></summary>

**Rozwiązanie:**
```cpp
void lock_free_push(int value) {
    Node* new_node = new Node(value);
    
    // Krok 1: Pobierz aktualną głowę
    new_node->next = head.load(); 
    
    // Krok 2: Pętla CAS. Próbujemy przepiąć head na new_node.
    // Metoda compare_exchange_weak odświeża pierwszy argument (new_node->next)
    // ładując do niego nową wartość head, jeśli CAS się nie powiódł!
    while (!head.compare_exchange_weak(new_node->next, new_node)) {
        // Pętla kręci się do skutku. Jeśli head ulegnie zmianie przez inny wątek,
        // new_node->next automatycznie zacznie wskazywać na ten nowy head,
        // a w kolejnej iteracji pętli spróbujemy ponownie.
    }
}
```

**Dowód poprawności (Linearyzowalność):**
Operacja współbieżna uznawana jest za linearyzowalną, jeśli można wskazać jeden, niepodzielny punkt w czasie, w którym struktura danych poprawnie przyjęła nowy stan z punktu widzenia każdego innego wątku.
1. Punktem linearyzacji dla procedury `lock_free_push` jest udane wykonanie instrukcji sprzętowej CAS na wskaźniku `head`.
2. Niech $H_i$ oznacza stan głowy stosu. Wątek T1 przygotowuje węzeł $N_{nowy}$, gdzie $N_{nowy} \rightarrow next = H_{stary}$.
3. Gdy Wątek T1 pomyślnie przechodzi przez CAS, mamy absolutną gwarancję na poziomie sprzętu, że stan globalny `head` w tamtej mikrosekundzie wynosił wciąż $H_{stary}$. 
4. Zatem struktura zmienia się w jednym atomowym kroku ze stanu $[H_{stary}, ...]$ na $[N_{nowy}, H_{stary}, ...]$.
5. Każdy inny wątek T2 próbujący w tym samym czasie zrobić `push`, którego CAS wykona się o nanosekundę później, zaliczy tzw. *failure* operacji CAS. Jego własny `expected` zostanie nadpisany nowym adresem $N_{nowy}$, co zmusi go w następnej iteracji do ustawienia wskaźnika swojego elementu na $N_{nowy}$. Żaden wstawiany element nie zostaje "nadpisany" ani zgubiony.
</details>


### Zadanie 5: Licznik z limitem (Capped Counter) (Poziom: Średni)
Standardowa metoda `fetch_add` pozwala na atomowe dodawanie, ale nie potrafi zatrzymać się na określonym limicie. Wyobraź sobie system rezerwacji biletów (np. limit to 100 biletów). Wiele wątków próbuje inkrementować licznik, ale nie może on przekroczyć wartości `limit`.

Napisz funkcję, która zwiększa licznik o 1 tylko, jeśli jest on mniejszy niż limit. Zwraca `true`, jeśli udało się zwiększyć, lub `false`, jeśli osiągnięto limit.

```cpp
bool increment_if_less_than(std::atomic<int>& counter, int limit) {
    // TODO: Zaimplementuj używając CAS
}
```

<details>
<summary><b>Kliknij, aby zobaczyć rozwiązanie i formalny dowód poprawności</b></summary>

**Rozwiązanie:**
```cpp
bool increment_if_less_than(std::atomic<int>& counter, int limit) {
    int old_val = counter.load();
    do {
        if (old_val >= limit) {
            return false; // Limit osiągnięty, przerywamy pętlę
        }
    } while (!counter.compare_exchange_weak(old_val, old_val + 1));
    return true;
}
```

**Dowód poprawności:**
1. **Ograniczenie (Safety):** W każdym punkcie czasu $t$, wartość licznika $C(t) \le limit$. Zmiana stanu z $C$ na $C+1$ może zajść wyłącznie wtedy, gdy wykonana zostanie instrukcja CAS. Bezpośrednio przed wywołaniem CAS dokonywane jest sprawdzenie `old_val < limit`. Ponieważ CAS powodzi się tylko wtedy, gdy stan globalny jest dokładnie równy `old_val`, gwarantuje to, że nowa wartość $C+1 \le limit$. Żaden wątek nie zdoła podbić licznika powyżej limitu (np. z 100 na 101).
2. **Brak utraconych biletów (No lost updates):** Linearyzowalność CAS zapewnia, że jeśli dwa wątki jednocześnie zobaczą `old_val = 99`, tylko jeden z nich z powodzeniem wykona CAS (ustawiając na 100). Drugi zanotuje porażkę (jego `old_val` zostanie zaktualizowane do 100), w kolejnej iteracji pętli warunek `old_val >= limit` (100 >= 100) będzie prawdziwy i wątek zwróci `false`.
</details>

---

### Zadanie 6: Własny Spinlock (Wirująca blokada) (Poziom: Średni)
Mutexy usypiają wątki, co jest wolne dla bardzo krótkich sekcji krytycznych. Zamiast tego możemy stworzyć `Spinlock` – blokadę, w której wątek aktywnie „kręci się” w pętli `while`, próbując założyć kłódkę przy pomocy CAS. 

Zaimplementuj metody `lock` i `unlock` operujące na zmiennej `std::atomic<bool>`.

```cpp
class Spinlock {
    std::atomic<bool> locked{false};
public:
    void lock() {
        // TODO: Zablokuj (kręć się w pętli, dopóki locked == true, użyj CAS)
    }

    void unlock() {
        // TODO: Odblokuj
    }
};
```

<details>
<summary><b>Kliknij, aby zobaczyć rozwiązanie i formalny dowód poprawności</b></summary>

**Rozwiązanie:**
```cpp
class Spinlock {
    std::atomic<bool> locked{false};
public:
    void lock() {
        bool expected = false;
        // Pętla obraca się dopóki CAS nie zdoła zmienić false na true.
        // Pamiętaj: compare_exchange_weak modyfikuje `expected` na wartość z pamięci 
        // przy niepowodzeniu, więc musimy w każdej iteracji resetować expected = false!
        while (!locked.compare_exchange_weak(expected, true)) {
            expected = false; 
            // Opcjonalnie w prawdziwym kodzie: _mm_pause() (x86) lub std::this_thread::yield()
        }
    }

    void unlock() {
        locked.store(false);
    }
};
```

**Dowód poprawności (Wzajemne wykluczanie - Mutual Exclusion):**
1. Aby wątek T1 mógł opuścić metodę `lock()` i wejść do sekcji krytycznej, musi pomyślnie wykonać instrukcję CAS z wartości `false` na `true`. W tym momencie punktu linearyzacji zmienna globalna `locked` przyjmuje wartość `true`.
2. Każdy kolejny wątek T2, który spróbuje wykonać `lock()`, wykona CAS próbujący zmienić `false` na `true`. Jednak wartość w pamięci to już `true`. Operacja CAS z `expected=false` zwróci błąd, a T2 pozostanie w pętli `while`.
3. Jedyną metodą zmiany stanu z powrotem na `false` jest wywołanie `unlock()`. Wykona je wątek T1, który aktualnie jest w sekcji krytycznej.
4. Wynika z tego, że w dowolnym momencie czasu co najwyżej jeden wątek może znajdować się pomiędzy powrotem z metody `lock()` a wejściem do `unlock()`.
</details>

---

### Zadanie 7: Leniwa Inicjalizacja Bez Blokad (Lazy Initialization) (Poziom: Trudny)
Bardzo popularny wzorzec w projektowaniu (Singleton). Mamy ciężki w inicjalizacji obiekt (np. połączenie z bazą danych lub duży cache), do którego dostęp trzymany jest w zmiennej `std::atomic<Cache*> global_cache{nullptr}`. 
Chcemy, by obiekt został utworzony dopiero wtedy, gdy po raz pierwszy zajdzie taka potrzeba. Wiele wątków może jednocześnie zauważyć `nullptr` i spróbować go stworzyć. Zapewnij, że globalny wskaźnik ustawi się poprawnie bez utraty (wycieku) pamięci.

```cpp
struct Cache { /* Ciężki obiekt */ };
std::atomic<Cache*> global_cache{nullptr};

Cache* get_or_init_cache() {
    // TODO: Zwróć global_cache. Jeśli to nullptr, utwórz go w sposób lock-free.
}
```

<details>
<summary><b>Kliknij, aby zobaczyć rozwiązanie i formalny dowód</b></summary>

**Rozwiązanie:**
```cpp
Cache* get_or_init_cache() {
    Cache* current = global_cache.load();
    if (current != nullptr) {
        return current; // Szybka ścieżka (fast path)
    }

    // Wielu wątkom mogło się wydawać, że jest nullptr.
    // Tworzymy naszą lokalną instancję.
    Cache* new_cache = new Cache();
    
    // Próbujemy wstawić naszą instancję jako globalną.
    // (Używamy wersji strong, by uniknąć spurious failures po alokacji pamięci)
    Cache* expected = nullptr;
    if (global_cache.compare_exchange_strong(expected, new_cache)) {
        // Sukces! My pierwsi wstawiliśmy obiekt.
        return new_cache;
    } else {
        // Porażka. Ktoś inny nas uprzedził i wsadził swój obiekt.
        // Musimy zniszczyć naszą niepotrzebną już kopię, by uniknąć wycieku.
        delete new_cache; 
        
        // Zwracamy to, co wgrał ten "ktoś inny" (zmienna expected zaktualizowała się
        // automatycznie na adres obiektu, który wygrał wyścig).
        return expected;
    }
}
```

**Dowód poprawności:**
1. **Bezpieczeństwo inicjalizacji:** `global_cache` początkowo to `nullptr`. Instrukcja CAS powiedzie się tylko pod warunkiem, że w momencie jej wykonania globalny wskaźnik to wciąż `nullptr`. W związku z tym, dokładnie **jeden** wątek zmieni wartość z `nullptr` na swój wskaźnik. Nie ma możliwości, by globalny wskaźnik został nadpisany dwukrotnie.
2. **Brak wycieków pamięci (Memory Safety):** Wątki "przegrane" w wyścigu otrzymają z CAS wartość `false`. Wejdą w blok `else`, w którym niszczą lokalnie zaalokowaną instancję `delete new_cache`. Zatem wszystkie nieużyte kopie zostaną bezpiecznie usunięte z pamięci sterty.
3. **Prawidłowy zwrot:** Wątki przegrane, w wyniku działania `compare_exchange_strong`, uzyskują w zmiennej `expected` aktualną zawartość pamięci, czyli wskaźnik instancji, która wygrała wyścig. Dzięki temu każdy wątek prawidłowo otrzymuje ten sam adres i współdzieli jeden obiekt globalny.
</details>

---

### Zadanie 8: Atomowa Maska Bitowa (Poziom: Średni)
Często w grach lub systemach wbudowanych obiekt ma zmienną określającą jego stany jako zestaw flag bitowych (np. bit 0: IN_COMBAT, bit 1: POISONED, bit 2: STUNNED). Dwa wątki mogą chcieć równocześnie nadać postaci różne statusy (jeden nadaje POISONED, drugi STUNNED).
Zwykłe `flags |= NEW_FLAG` nie jest atomowe i może zgubić status. Napisz atomową wersję funkcji `set_flag(std::atomic<uint32_t>& flags, uint32_t flag_to_set)`.

```cpp
void set_flag(std::atomic<uint32_t>& flags, uint32_t flag_to_set) {
    // TODO: Zaimplementuj używając CAS loop
}
```

<details>
<summary><b>Kliknij, aby zobaczyć rozwiązanie i formalny dowód</b></summary>

**Rozwiązanie:**
*(Notka: Biblioteka standardowa posiada `fetch_or`, ale ćwiczymy ręczny CAS, który bywa niezbędny przy bardzo złożonych manipulacjach wieloma bitami naraz, np. włącz bit A ale tylko jeśli wyłączony jest bit B).*

```cpp
void set_flag(std::atomic<uint32_t>& flags, uint32_t flag_to_set) {
    uint32_t old_flags = flags.load();
    uint32_t new_flags;
    do {
        new_flags = old_flags | flag_to_set;
        // Jeśli flaga już tam jest, nie musimy robić drogiego zapisu
        if (old_flags == new_flags) {
            break;
        }
    } while (!flags.compare_exchange_weak(old_flags, new_flags));
}
```

**Dowód poprawności:**
1. Operacja odczytu, modyfikacji (nałożenia maski poprzez sumę logiczną `|`) oraz zapisu jest zamknięta w pętli uwarunkowanej sprzętowym CAS. Oznacza to brak tzw. *Lost Updates*.
2. Jeśli Wątek 1 (W1) ustawia bit 0 (`flag_to_set = 1`), a Wątek 2 (W2) ustawia bit 1 (`flag_to_set = 2`) i działają w tym samym momencie na `old_flags = 0`:
   * W1 wylicza `new_flags = 1`.
   * W2 wylicza `new_flags = 2`.
   * Jeśli W1 pierwszy zaaplikuje CAS, stan globalny to `1`.
   * W2 spróbuje CAS (`old=0`, `new=2`). CAS zwróci błąd, a do zmiennej W2 `old_flags` zostanie załadowane `1` (wynik pracy W1).
   * W2 wchodzi w kolejną iterację, wylicza `new_flags = 1 | 2 = 3` (czyli oba bity). Wykonuje CAS z sukcesem.
   * Końcowy stan `flags` to `3` (oba statusy nakładają się bezkolizyjnie), czego nie gwarantowałaby zwykła operacja asynchroniczna na typach nieatomowych.
</details>
