NWD ( największy wspólny dzielnik ), inaczej algorytm Euklidesa - dla liczb całkowitych jest to największa liczba całkowita, która dzieli podane liczby bez reszty. 
Na przykład, NWD(14, 28, 56) = 14, ponieważ 14 jest największą liczbą, która dzieli zarówno 14, 28, jak i 56 bez reszty. Łatwo to zauważyć, ponieważ liczby 28 i 56 są 
wielokrotnościami liczby 14, przez co ona jest rozwiązaniem NWD(14, 28, 56)

Rozszerzony algorytm Euklidesa - algorytm który podczas oblicznia NWD z podanych liczb, liczy również współczynniki (liczby całkowite), które pozwalają na zapisanie
NWD jako kombinacji liniowej tych liczb. NWD(a, b) = x*a +y*b.

Przykład:
NWD (546, 308) = -9 * 546 + 16 * 308

Algorytm ten działa następująco:

Dla danych dwóch liczb całkowitych a i b, algorytm Euklidesa znajduje NWD(a, b) poprzez iteracyjne obliczanie reszty z dzielenia a przez b, 
a następnie zamianę miejsc a na b i b na resztę. Algorytm kontynuuje te kroki, aż reszta stanie się równa zero. Wtedy NWD(a, b) to ostatnia niezerowa reszta.
Rozszerzony algorytm Euklidesa rozszerza ten proces o obliczanie współczynników Bézouta. W każdym kroku algorytmu, oprócz aktualizacji reszty, 
oblicza się także współczynniki Bézouta x i y z równania ax + by = NWD(a, b). Współczynniki Bézouta są obliczane rekurencyjnie na podstawie 
współczynników obliczonych w poprzednich krokach algorytmu.

############################################################################################
############################################################################################
############################################################################################

/*
 * @file euclidean_algorithm.hpp
 * Implementacja rozszerzonego algorytmu Euklidesa w C++20 przy użyciu modułów.
 */

############################################################################################

	template <std::integral... Is>
	struct euclidean_result {
        std::tuple<Is...> coefficients{}; /< Współczynniki Bézouta
        std::uint64_t GCD{};              /< Największy wspólny dzielnik
    };

    /*
     * Struktura przechowująca wynik działania rozszerzonego algorytmu Euklidesa.
     * 
     * Ta struktura przechowuje współczynniki Bézouta oraz największy wspólny dzielnik (NWD) 
     * obliczone przez rozszerzony algorytm Euklidesa.
     */

############################################################################################

	template <std::integral I, std::integral J>
	constexpr auto gcd_extended(I i, J j) -> euclidean_result<I, J>;

    /*
     * Implementacja rozszerzonego algorytmu Euklidesa.
     * 
     * Ta funkcja oblicza największy wspólny dzielnik (NWD) oraz współczynniki Bézouta
     * dwóch liczb całkowitych przy użyciu rozszerzonego algorytmu Euklidesa.
     * 
     * @parametr i => Pierwsza liczba całkowita.
     * @parametr j => Druga liczba całkowita.
     * @zwraca => Instancja `euclidean_result` zawierająca współczynniki Bézouta i NWD.
     */

############################################################################################

	
    template <std::integral I, std::size_t size>
    constexpr auto gcd_extended(std::array<I, size> is) -> std::pair<std::array<std::int64_t, size>, std::uint64_t>;

    /*
     * Implementacja rozszerzonego algorytmu Euklidesa dla tablicy, dla więcej niż 2 elementów.
     * 
     * Ta funkcja oblicza największy wspólny dzielnik (NWD) oraz współczynniki Bézouta
     * tablicy liczb całkowitych przy użyciu rozszerzonego algorytmu Euklidesa.
     * 
     * @parametr is => Tablica liczb całkowitych.
     * @zwraca => Para zawierająca tablicę współczynników Bézouta i NWD.
     */


############################################################################################


    /*
     * Obiekt wywoływalny do obliczania NWD i współczynników Bézouta wielu liczb.
     * 
     * Ta struktura zapewnia obiekt wywoływalny do obliczania największego wspólnego dzielnika (NWD) 
     * i współczynników Bézouta wielu liczb przy użyciu rozszerzonego algorytmu Euklidesa.
     */

    struct euclidean {

        /*
         * Oblicza NWD i współczynniki Bézouta wielu liczb.
         * 
         * Ta funkcja oblicza NWD i współczynniki Bézouta wielu liczb
         * przy użyciu rozszerzonego algorytmu Euklidesa.
         * 
         * @tymczasowe parametry Is => Parametry szablonu zmiennej reprezentujące liczby całkowite.
         * @parametr is => Liczby całkowite.
         * @zwraca => Instancja `euclidean_result` zawierająca współczynniki Bézouta i NWD.
         */
        template <std::integral... Is>
        requires (sizeof...(Is) >= 2)
        constexpr auto operator()(Is... is) -> euclidean_result<Is...>;

############################################################################################

	template <std::integral... Is>
        auto show_steps(Is... is) -> std::string;	

        /*
         * Pokazuje i wyjaśnia kolejne kroki algorytmu Euklidesa.
         * 
         * Ta funkcja tworzy ciąg znaków wizualizujący kolejne kroki algorytmu Euklidesa
         * wykonywane na podanych liczbach.
         * 
         * @tymaczasowe parametry Is => Parametry szablonu zmiennej reprezentujące liczby całkowite.
         * @parametr is => Liczby całkowite.
         * @zwraca => Ciąg znaków wizualizujący kolejne kroki algorytmu Euklidesa.
         */

############################################################################################

STRUKTURA euclidian JEST TYM SAMYM CO JEST OPISANE POWYŻEJ Z WYJĄTKIEM POKAZYWNIA KROK PO KROKU JAK POWINNO SIĘ ROZWIĄZYWAĆ DANE ZAGADNIENIE

############################################################################################

PRZYKŁADOWY TEST: 
Z liczb 546 oraz 308 powinny wyjść wyniki: 
-NWD (546, 308) = 14
- Współczynniki: x = -9 oraz y = 16
