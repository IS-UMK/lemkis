// USE C++20 MODULES
// READ e.g. https://learn.microsoft.com/en-us/cpp/cpp/modules-cpp?view=msvc-170

//Podpinanie bibliotek
#include <concepts>
#include <tuple>
#include <string>
#include <algorithm>
#include <numeric>
#include <ranges>
#include <iostream>

namespace algorithms 
{
	template <std::integral... Is>
	struct euclidean_result //Struktura zawierająca współczynniki oraz GCD(NWD)
	{ 
		std::tuple<Is...> coefficients{}; //Współczynniki
		std::uint64_t GCD{}; //GDC
	};

	template <std::integral I, std::integral J>
	constexpr auto gcd_extended(I i, J j) -> euclidean_result<I, J> //Funkcja obliczająca współczynniki oraz GCD dla 2 wartości
	{
		euclidean_result<I, J> result{};
		int x = 0, y = 1, x_prev = 1, y_prev = 0; //Określenie początkowych współczynników 
		auto a{ i }; //Liczba podana do obliczeń
		auto b{ j }; //Liczba podana do obliczeń
		while (b != 0) 
		{
			int quotient = a / b; //Sprawdzanie ile razy b zmieści się w a
			int temp = b;
			b = a % b; //Obliczanie reszty z dzielenia
			a = temp; //Podmiana miejsc zmiennych a i b

			int temp_x = x;
			x = x_prev - quotient * x; //Obliczanie współczynnika stojącego przy pierwszej podanej liczbie
			x_prev = temp_x;

			int temp_y = y;
			y = y_prev - quotient * y; //Obliczanie współczynnika stojącego przy drugiej podanej liczbie
			y_prev = temp_y;
		}

		//Funkcja tutaj zwraca współczynniki razem z GCD
		result.coefficients = std::tuple{ x_prev, y_prev };
		result.GCD = a;
		return result;
	}

	template <std::integral I, std::size_t size>
		requires (size >= 2) //Ustawienie warunku
	constexpr auto gcd_extended(std::array<I, size> is) -> std::pair <std::array<std::int64_t, size>, std::uint64_t> //Funkcja obliczająca współczynniki oraz GCD dla wielu wartości
	{
		if constexpr (size == 2) 
		{
			auto [coeff, gcd] = gcd_extended(is[0], is[1]); //Przypisanie elemetnów talicy do zmiennych, na których program oblicza
			auto [x, y] = coeff; //Inicjacja współczynników
			return std::pair{ std::array<std::int64_t, 2>{x, y}, gcd }; //Zwracanie obliczonych wartości
		}
		else 
		{
			std::array<I, size - 1> all_but_last{}; //Deklaracja tablicy liczb do programu
			for (auto [i, el] : is | std::views::take(size - 1) | std::views::enumerate) //Sprawdzenie jak duża tablica musi być
			{
				all_but_last[i] = el;
			}

			auto [coeff, gcd] = gcd_extended(all_but_last); //Wywołanie funkcji dla wszystkich elementów tablicy bez ostatniego i przekazanie wyników to zmiennych
			auto [coeff2, gcd2] = gcd_extended(gcd, is[size - 1]); //Wywołanie funkcji dla ostatniego i przedostatniego współczynnika i przypisanie tego do zmiennych
			auto [alpha, beta] = coeff2; //Przypisanie współczynników alpha i beta z wyników coeff2
			std::array<std::int64_t, size> x{}; //Deklaracja tablicy dla współczynników
			for (auto [ind, c] : coeff | std::views::enumerate) 
			{
				x[ind] = alpha * c; //Oblicznie współczynnika oraz przypisanie go do tablicy
			}
			x[size - 1] = beta; //Przypisanie wartości beta do przedostatniego elementu tablicy
			return std::pair{ x, gcd2 }; //Zwracanie współczynników
		}
	}
	struct euclidean {

		auto array_to_tuple(auto arr) {
			return std::apply([](auto ... vals) {return std::tuple{ vals... }; }, arr);
		}
	public:
		/* calculates GCD greatest common divisor of given numbers n_1, n_2, ...,
		n_k, where k = sizeof...(Is) and coefficients x_1, x_2, ..., x_k such
		that x_1 n_1 + x_2 n_2 + ... + x_k n_k = GCD */
		template <std::integral... Is>
			requires (sizeof...(Is) >= 2)
		constexpr auto operator()(Is...is)->euclidean_result<Is...> {
			auto [coeff, gcd] = algorithms::gcd_extended(std::array{ is... });
			return euclidean_result<Is...> {.coefficients{ array_to_tuple(coeff) }, .GCD{ gcd }};
		}

	public:
		/* shows and explains consecutive steps used in the Euclidean algorithm
		(and any other techniques used) performed on the given numbers. Produces
		a string which visualizes these steps. */

		template <std::integral... Is>
			requires (sizeof...(Is) >= 2)
		auto showSteps(Is... nums) -> std::string {
			std::ostringstream steps;
			steps << "kroki rozszerzonego algorytmu euklidesa:\n";
			std::array<int, sizeof...(Is)> numbers{ nums... };
			constexpr size_t N = sizeof...(Is);

			auto [coeff, gcd] = gcdExtended(numbers);
			steps << "wyliczenie najwiekszego wspolnego dzielnika: ";
			((steps << nums << ", "), ...);
			steps << "\n\n";

			for (size_t i = 0; i < N - 1; ++i) {
				std::cout << steps << "Step: " << i + 1 << "\n";
				steps << "GCD(" << numbers[i] << ", " << numbers[i + 1] << ")\n";

				// Perform the extended Euclidean algorithm for the current pair of numbers
				auto [coeff, current_gcd] = gcdExtended(numbers[i], numbers[i + 1]);

				// Extract coefficients
				auto [x, y] = coeff;

				// Display coefficients
				steps << "  Wspolczynniki (x, y) for GCD: (" << x << ", " << y << ")\n";

				// Update numbers array for the next iteration
				numbers[i + 1] = current_gcd;

				// Display the updated numbers array
				steps << "  Zaaktualizowane elementy tablicy: ";
				for (size_t j = 0; j < N; ++j) {
					steps << numbers[j];
				};
			}
		}
	};
}

// REMEMBER TO PROVIDE SOME TESTS AND EXAMPLES. IN README, PROVIDE
// INFORMATION ABOUT HOW TO LAUNCH YOUR PROGRAM. MOREOVER, DESCRIBE HOW THE
// UNDERLYING ALGORITHMS WORK. PROVE FACTS WHICH WERE NOT SHOWN DURING OUR
// CLASSES.

// REMEMBER TO STICK TO PROGRAMMING RULES SEE E.G.
// https://google.github.io/styleguide/cppguide.html

// You might find useful std::gcd, std::lcm, std::ranges::right_fold_last
// https://stackoverflow.com/questions/147515/least-common-multiple-for-3-or-more-numbers,
// https://stackoverflow.com/questions/16628088/euclidean-algorithm-gcd-with-multiple-numbers

// You can use recursion but only tail recursion (see
// https://stackoverflow.com/questions/33923/what-is-tail-recursion and
// https://stackoverflow.com/questions/34125/which-if-any-c-compilers-do-tail-recursion-optimization)