


### Transform Product
Testuje operację x → x² (każdy element do kwadratu) na wszystkich elementach input i zapisuje wynik do output.

Implementacja			Opis

* std::transform (seq)		Sekwencyjna wersja — brak równoległości

* std::transform (par)		Równoległa wersja przy użyciu execution::par

* std::transform (par_unseq)	Równoległa + wektorowana wersja ( Proces, w którym wiele danych jest przetwarzanych równocześnie w jednym cyklu procesora przy użyciu instrukcji SIMD (Single Instruction, Multiple Data). To oznacza, że procesor wykonuje tę samą operację na wielu elementach danych jednocześnie, zamiast przetwarzać je jeden po drugim.)

* OpenMP transform		Ręczne zrównoleglenie pętli for z #pragma omp parallel for

### Dot Product
Liczy sumę a[i] * b[i] dla wszystkich i.

Implementacja			Opis

* std::inner_product (seq)	Klasyczna sekwencyjna funkcja z STL(Standard Template Library - działa jedno-wątkowo (czyli sekwencyjnie — po kolei, krok po kroku))
* std::transform_reduce (par)	Wbudowany algorytm z równoległością
* std::transform_reduce (par_unseq)	Równoległy i wektorowany
* OpenMP dot product	Ręczne zrównoleglenie z redukcją (reduction(+:res))
