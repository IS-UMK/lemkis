# Współbieżność w Systemach Wbudowanych
## Systemy Operacyjne Czasu Rzeczywistego dla Mikrokontrolerów

## Definicje Podstawowe

### Systemy wbudowane
Wyspecjalizowane systemy komputerowe, wykonujące dedykowane funkcje lub zadania w ramach większego systemu mechanicznego lub elektrycznego.

### Systemy krytyczne
Systemy, których awaria lub usterka może skutkować poważnymi konsekwencjami, takimi jak utrata życia, znaczne szkody materialne, środowiskowe, czy inne.

### Systemy czasu rzeczywistego
Systemy wykonujące zadania z rygorystycznymi wymaganiami czasowymi, w sytuacjach, w których opóźnienia lub nieregularne czasy reakcji mogą doprowadzić do załamania całego systemu.

## Rodzaje Systemów Czasu Rzeczywistego

### "Hard" real-time
Przekroczenie deadline'u może być katastrofalne w skutkach. Reakcja systemu musi nastąpić w ściśle określonym czasie.

### "Soft" real-time
Sporadyczne przekroczenie deadline'u jest niepożądane, ale tolerowalne. System pozostaje użyteczny pomimo opóźnień.

## Problemy na Poziomie Projektowania Systemu

### Wymagania Czasowe

#### Terminy
Precyzyjne określenie, jakie zadania muszą być wykonane w określonym czasie i jakie są konsekwencje przekroczenia terminów.

#### Opóźnienia
Minimalizacja opóźnień w przetwarzaniu i reakcji systemu na zdarzenia zewnętrzne.

#### Jitter
Zapewnienie przewidywalności czasów odpowiedzi. Unikanie nieregularnych opóźnień.

### Planowanie i Harmonogramowanie

#### Algorytmy Planowania
Właściwy dobór algorytmu planowania zadań zgodnie z potrzebami oraz wymaganiami wobec systemu.

#### Analiza Wykonalności
Formalna analiza tego, czy wszystkie zadania mogą być wykonane w zakładanych terminach przy dostępnym obciążeniu procesora.

#### Priorytety Zadań
Konieczne jest właściwe ustalenie hierarchii zadań, aby uniknąć sytuacji, w której zadania o niższym priorytecie blokują krytyczne zadania (np. inwersja priorytetów).

### Zarządzanie Zasobami

#### Zakleszczenia i Wyścigi
Przewidywanie potencjalne konflikty o współdzielone zasoby i właściwie zastosować mechanizmy synchronizacji i dziedziczenia priorytetów.

#### Minimalne Współdzielenie Zasobów
Ograniczenie dostępu do współdzielonych zasobów, aby zmniejszyć ryzyko opóźnień.

### Determinizm
System musi działać w sposób przewidywalny, niezależnie od obciążenia czy warunków zewnętrznych. Trzeba uwzględnić najgorszy przypadek (Worst-Case Execution Time) dla każdego zadania.

## Realizacja Systemów Hard Real-Time

Poprzez połączenie dedykowanego hardware'u i specjalnego software'u.

Nie ma lepszego sposobu na realizowanie zadań real-time niż dedykowany sprzęt (jeśli oczywiście w ogóle sprzętowa realizacja zadania jest możliwa).

### Hardware
- **ASICs** - Application-Specific Integrated Circuits
- **FPGAs** - Field-Programmable Gate Arrays
- **DSPs** - Digital Signal Processors
- **NPUs/TPUs** - Neural/Tensor Processing Units
- **GPGPUs** - General Purpose Graphics Processing Units

### Software
- **Aerospace**: VxWorks, INTEGRITY, RTEMS
- **Automotive**: AUTOSAR
- **Ogólne**: RTL (Real-Time Linux), FreeRTOS, Zephyr, etc.

## Systemy Operacyjne Czasu Rzeczywistego

RTOS to konstrukt umożliwiający współbieżne wykonywanie zadań, zaprojektowany w celu bardziej przewidywalnego i niezawodnego wykonywania zadań w ściśle określonych terminach za pomocą specjalnych algorytmów szeregowania zadań oraz (najczęściej) mechanizmu wywłaszczania. Zapewnia również mechanizmy i struktury do synchronizacji zadań, ochrony danych i wymiany informacji między zadaniami.

### Cechy charakterystyczne:
- **Deterministyczne zachowanie**
- **Precyzyjne timery**
- **Minimalny jitter**
- **Szybkie przełączanie**

## Algorytmy Szeregowania Zadań w RTOSach

### Pojęcia i Metryki

**J** - zbiór zadań, **n = |J|** - liczebność zbioru zadań

#### Podstawowe definicje:
- **Czas przybycia: aⱼ** - Moment, w którym zadanie Jⱼ staje się gotowe do wykonania
- **Czas zakończenia: fⱼ** - Moment zakończenia zadania
- **Absolutny termin: dⱼ** - Czas, do którego zadanie powinno być ukończone
- **Opóźnienie: Lⱼ** - Czas, przez który zadanie pozostaje aktywne po terminie
  - Lⱼ = fⱼ - dⱼ
- **Zwłoka: Eⱼ** - Najgorszy przypadek opóźnienia
  - Eⱼ = max(0, Lⱼ)
- **Luz: Xⱼ** - Maksymalny czas, o który zadanie może być opóźnione i nadal dotrzymać terminu
  - Xⱼ = dⱼ - aⱼ - Cⱼ, gdzie (Cⱼ - czas wykonania)

### Rodzaje terminów

#### Termin implicytny
- Termin Dⱼ jest równy okresowi Tⱼ: **Dⱼ = Tⱼ**
- Każde zadanie musi być ukończone przed rozpoczęciem kolejnego cyklu (np. zadanie o okresie 50 ms ma termin 50 ms od przybycia)
- Typowe dla systemów, gdzie zadania muszą być regularnie wykonywane, np. w RMS i EDF

#### Termin ograniczony
- Termin Dⱼ jest mniejszy lub równy okresowi Tⱼ: **Dⱼ ≤ Tⱼ**
- Zadanie musi być ukończone wcześniej niż w kolejnym cyklu. Zwiększa to wymagania czasowe
- Przydatne w systemach wymagających szybszej odpowiedzi

#### Termin dowolny
- Termin Dⱼ może być większy niż okres Tⱼ: **Dⱼ > Tⱼ**
- Zadanie może mieć dłuższy termin niż jego okres, co jest rzadsze i komplikuje planowanie, ponieważ zadania mogą się nakładać na kolejne cykle

### Metryki oceny algorytmów

#### Average Response Time
$$\frac{\sum_{j \in J} (f_j - a_j)}{|J|}$$

Średnia długość czasu od momentu przybycia zadania. Jak szybko algorytm obsługuje zadania.

#### Makespan
Czas od przybycia najwcześniejszego zadania do zakończenia ostatniego zadania. Całkowity czas potrzebny na wykonanie wszystkich zadań.

#### Total Weighted Response Time
$$\sum_{j \in J} w_j(f_j - a_j)$$

Suma czasów odpowiedzi zadań, ważona zgodnie z priorytetem wⱼ. Stopień priorytetowego traktowania ważniejszych zadań.

#### Maximum Lateness
Największe opóźnienie zadania względem jego terminu. Wskazuje najgorszy przypadek przekroczenia terminu.

#### Number of Late Jobs
$$N_{late} = \sum_{j \in J} miss(J_j)$$

gdzie $miss(J_j) = 1 \Leftrightarrow f_j > d_j$

Liczba zadań, które zakończyły się po terminie. Mierzy niezawodność algorytmu w dotrzymywaniu terminów.

## Główne Algorytmy Szeregowania

### Rate-Monotonic Scheduling (RMS)
- **Stałe Priorytety**
- **Wywłaszczanie**

Planowanie RMS przydziela stałe priorytety zadaniom na podstawie ich okresu, gdzie krótszy okres (większa częstotliwość występowania) oznacza wyższy priorytet. Zapewnia przewidywalność i prostotę. Działa dobrze dla okresowych zadań z implicytnymi terminami, ale ogranicza wykorzystanie procesora do około 69% w przypadku wielu zadań i nie jest elastyczny w przypadku nieimplicytnych oraz zmiennych terminów.

### Deadline Monotonic Scheduling (DM)
- **Stałe Priorytety**
- **Wywłaszczanie**

Planowanie DM przypisuje stałe priorytety na podstawie względnych terminów - maksymalnego czasu od momentu przybycia zadania do momentu, gdy musi być zakończone - gdzie krótszy termin daje wyższy priorytet. Jest prosty i przewidywalny jak RMS, ale lepiej radzi sobie z różnorodnymi wymaganiami czasowymi. Odpowiedni dla systemów, w których terminy różnią się od okresów. Nie wymaga terminów implicytnych.

### Earliest Deadline First (EDF)
- **Dynamiczne Priorytety**
- **Wywłaszczanie**

Planowanie EDF dynamicznie przydziela priorytety zadaniom na podstawie najbliższego absolutnego terminu, wybierając zadanie z najwcześniejszym terminem do wykonania. Jest to algorytm wywłaszczający, teoretycznie optymalny, umożliwiający 100% wykorzystanie procesora dla zadań okresowych z implicytnymi terminami. Ale zarządzanie priorytetami zwiększa narzut obliczeniowy.

### Ocena algorytmów

Wcześniej wspomniane metryki pozwalają ilościowo ocenić wydajność algorytmów planistycznych, takich jak RMS, DM, czy EDF:

- **Responsywność**: Średni czas odpowiedzi i całkowity ważony czas odpowiedzi pokazują, jak szybko zadania są obsługiwane
- **Efektywność**: Makespan ocenia, jak dobrze algorytm wykorzystuje czas procesora
- **Niezawodność**: Maksymalne opóźnienie i liczba spóźnionych zadań wskazują, czy algorytm dotrzymuje terminów
- **Priorytetyzacja**: Ważony czas odpowiedzi pozwala ocenić, czy algorytm faworyzuje ważniejsze zadania

## Systemy Operacyjne Czasu Rzeczywistego - Przykłady Realne

### Real-Time Linux

Real-Time Linux to zestaw poprawek dla kernela Linuxa wprowadzający do niego zmiany zorientowane na pracę w czasie rzeczywistym.

| Cecha | PREEMPT_DYNAMIC | PREEMPT_RT |
|-------|-----------------|------------|
| **Wywłaszczalność kernela** | Większość kodu kernela działa w sekcjach niewywłaszczalnych - zadanie w trybie kernela nie może zostać przerwane, dopóki nie wróci do przestrzeni użytkownika | Większość sekcji kernela staje się możliwa do wywłaszczenia - nawet kod w trybie kernela może zostać przerwany, jeśli nadchodzi wątek o wyższym priorytecie |
| **Obsługa przerwań** | Przerwania są obsługiwane przez handlery działające jako tzw. hard IRQ - kod uruchamiany jest natychmiast. Nie mogą być ani przełączane ani planowane. Mogą blokować inne operacje | Przenosi obsługę przerwań do wątków kernela - tzw. threaded IRQs - które można przerywać mechanizmem wywłaszczania i przypisywać im priorytety |
| **Blokady w kernelu** | Używa spinlocków, które aktywnie zajmują CPU w pętli oczekując na dostęp do zasobu i nie można ich przerwać i przełączyć kontekstu | Zastępuje spinlocki mutexami, dzięki czemu możliwe jest usypianie, nadawanie priorytetów i wywłaszczanie |
| **Dziedziczenie priorytetów** | Brak jest wymuszonego dziedziczenia - wątek niskiego priorytetu może blokować wyższy | Wspiera dziedziczenie priorytetu - wątek trzymający zasób tymczasowo dziedziczy wyższy priorytet, by uniknąć blokad |
| **Planowanie zadań** | Stosowanie planisty CFS, aby zapewnić "całkowitą sprawiedliwość" w przydziale czasu procesora i zasobów sprzętowych. Maksymalizacja przepustowości i wydajności kosztem mniejszego determinizmu i większych opóźnień | Deterministyczne planowanie. Wywłaszczalność niemal dowolnego zadania, włącznie z obsługą przerwań i kodem kernela. Dodatkowa optymalizacja polityk kolejkowania SCHED_FIFO i SCHED_RR pod niższe opóźnienia |
| **Liczniki czasu** | Do generowania przerwań czasowych wykorzystuje głównie zwykle timery o rozdzielczości 1-10 ms. Wspiera timery wysokiej rozdzielczości, ale rzadko wykorzystuje je w praktyce | Wykorzystuje timery wysokiej rozdzielczości specjalnie zoptymalizowane do precyzyjnego i deterministycznego odmierzania czasu, czasem mogące osiągać rozdzielczość nawet poniżej 1 μs |
| **Pozostałe** | Duże sekcje krytyczne, Duży jitter, Częste przerwania czasowe | Małe sekcje krytyczne, Mały jitter, Przerwania czasowe wyłączone |

> **Uwaga historyczna**: Nareszcie, po 20 latach rozwoju, poprawki czasu rzeczywistego w końcu zostały wprowadzone do głównej linii rozwojowej Linuxa!

### FreeRTOS

#### Perspektywa historyczna
FreeRTOS to darmowy, otwarto źródłowy system operacyjny czasu rzeczywistego zaprojektowany dla urządzeń wbudowanych. Znany jest ze swojej niezawodności, skalowalności, łatwości obsługi, kompatybilności oraz szerokiego zastosowania. Powstał w 2003 roku, a jego stworzenie było motywowane wysokimi kosztami licencji na systemy komercyjne oraz rosnącą mocą obliczeniową i możliwościami systemów wbudowanych. Obecnie udostępniany jest na licencji MIT, która zapewnia wysoki stopień swobody w jego użytkowaniu. W 2017 roku projekt został przejęty przez Amazon, który w odpowiedzi na dynamiczny rozwój internetu rzeczy rozszerzył jego funkcjonalność o wsparcie dla komunikacji sieciowej i kompatybilność z usługami chmurowymi AWS.

#### Perspektywa techniczna
FreeRTOS to lekka implementacja deterministycznego planisty czasu rzeczywistego opartego o mechanizm wywłaszczania i stałe priorytety zadań. Dla zadań o tym samym priorytecie obsługiwane jest szeregowanie round-robin z opcjonalnym time-slicing'iem. Opcjonalnie dostępne jest też planowanie kooperatywne. W swoim API dostarcza niezbędne funkcje do zarządzania wielozadaniowością, synchronizacji zadań oraz wymiany informacji między zadaniami.

#### Główni producenci wspierający FreeRTOS:
- STMicroelectronics
- Espressif
- NXP Semiconductors
