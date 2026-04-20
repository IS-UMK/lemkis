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