# Dokumentacja projektu: Reprezentacja Ułamków

## Przegląd
Projekt Reprezentacji Ułamków zapewnia funkcjonalności do reprezentacji 
ułamków oraz ich rozwinięć w różnych systemach liczbowych. Zawiera operacje 
do rozwijania ułamków na części całkowite, ułamkowe i okresowe, a także do 
odtwarzania ułamków z ich rozwiniętej reprezentacji.
## Moduły
### 1. representation
Ten moduł zawiera podstawową funkcjonalność do reprezentowania ułamków i 
rozwijania ich na składowe części.
#### 1.1 fraction
* Opis: Definiuje strukturę fraction do reprezentacji ułamka z 
całkowitym licznikiem i mianownikiem.
* Składowe:
o numerator: Całkowity licznik ułamka.
o denominator: Całkowity mianownik ułamka.
* Metody:
o operator-=: Odejmuje wartość całkowitą od ułamka.
#### 1.2 expansion
* Opis: Definiuje strukturę expansion do reprezentacji rozwinięcia 
ułamka w określonej podstawie.
* Składowe:
* * whole: Ciąg znaków reprezentujący część całkowitą rozwinięcia.
* * fractial: Ciąg znaków reprezentujący nieokresową część ułamkową 
rozwinięcia.
* * period: Ciąg znaków reprezentujący okresową część rozwinięcia.
#### 1.3 Funkcje
* expand_whole: Rozwija część całkowitą ułamka w określonej podstawie.
* expand_fractional_part: Rozwija nieokresową część ułamka.
* expand_period_part: Rozwija okresową część ułamka.
* expand: Łączy funkcje (expand_whole, expand_fractional_part, 
expand_period_part) w celu rozwinięcia ułamka na część całkowitą, 
ułamkową i okresową.
* dexpand_h: Konwertuje ciąg znaków reprezentujący liczbę w danej 
podstawie do jej reprezentacji całkowitej.
* pow: Oblicza potęgę liczby całkowitej.
* dexpand_whole_part: Odtwarza część całkowitą ułamka z jego 
rozwinięcia.
* dexpand_fractial_part: Odtwarza nieokresową część ułamka z jego 
rozwinięcia.
* dexpand_period_part: Odtwarza okresową część ułamka z jego 
rozwinięcia.
* dexpand: Łączy funkcje (dexpand_whole_part, dexpand_fractial_part, 
dexpand_period_part) w celu odtworzenia ułamka z jego rozwiniętej 
reprezentacji.
### 2. representation_tests
Ten moduł zawiera testy sprawdzające poprawność zaimplementowanych 
funkcji.
#### Użycie
* Użytkownicy mogą dołączyć niezbędne nagłówki z przestrzeni nazw 
representation do pracy z ułamkami i ich rozwinięciami.
* Dostarczane są różne operacje, takie jak rozwijanie ułamków i
odtwarzanie ułamków
#### Przykład
```
representation_tests();
```

Ta funkcja wywołuje przypadki testowe zdefiniowane w representation_tests, 
demonstrując użycie i funkcjonalność zaimplementowanych funkcji.
#### Kompatybilność
* Kod jest kompatybilny z kompilatorami obsługującymi koncepcje i 
funkcje C++20.
#### Uwaga
* Ta dokumentacja zapewnia przegląd projektu Reprezentacji Ułamków i 
jego składników.
* Zachęca się użytkowników do odwołania się do kodu źródłowego i 
komentarzy w celu uzyskania szczegółowych informacji i szczegółów 
implementacji.