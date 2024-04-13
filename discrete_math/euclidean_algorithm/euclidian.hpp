// USE C++20 MODULES
// READ e.g. https://learn.microsoft.com/en-us/cpp/cpp/modules-cpp?view=msvc-170

//libraries
#include <concepts>
#include <tuple>
#include <string>
#include <algorithm>
#include <numeric>
#include <ranges>
#include <iostream>
#include <print>
#include <sstream>

namespace algorithms
{
	template <std::integral... Is>
	// Structure containing coefficients and greatest common divisor
	struct euclidean_result 
	{
		// Coefficients
		std::tuple<Is...> coefficients{}; 
		// Greatest common divisor
		std::uint64_t GCD{}; 
	};

	template <std::integral I, std::integral J>
	// Function calculating coefficients and gcd for 2 elements
	constexpr auto gcd_extended(I i, J j) -> euclidean_result<I, J> 
	{
		euclidean_result<I, J> result{};
		// Defining primary coefficients
		int x = 0, y = 1, x_prev = 1, y_prev = 0;  
		// Number given for calculation
		auto a{ i }; 
		// Number given for calculation
		auto b{ j };
		while (b != 0)
		{
			// Checking how many times b will fit into a
			int quotient = a / b; 
			std::println("a / b");
			std::println("Dividing number a by number b, result is {} \n", quotient);

			int temp = b;
			// Counting the rest from division
			b = a % b; 
			std::println("b = a % b");
			std::println("Then we count modulo (rest from division) numbers a and b, write it as number b ");
			// Switching up places of the variables a and b
			a = temp; 
			std::println("Switching up places of a and b, which were counted above");

			int temp_x = x;
			// Calculating the coefficient of the first given number
			x = x_prev - quotient * x; 
			std::println("We calculate coefficient x by subtraction from last coefficient's x product");
			x_prev = temp_x;
			std::println("Switching up places of previous coefficient x with temporary coefficient x, which were counted above");
			std::println("We do the same with coefficient y");

			int temp_y = y;
			// Calculating the coefficient of the second given number
			y = y_prev - quotient * y; 
			y_prev = temp_y;
		}

		// This function returns coefficients along with GCD
		result.coefficients = std::tuple{ x_prev, y_prev };
		result.GCD = a;
		return result;
	}

	template <std::integral I, std::size_t size>
		// Setting requirements
		requires (size >= 2) 
	// Function calculating coefficients and gcd for many elements
	constexpr auto gcd_extended(std::array<I, size> is) -> std::pair <std::array<std::int64_t, size>, std::uint64_t> 
	{
		if constexpr (size == 2)
		{
			// Assigning elements of an array to variables, that are used throughout the process
			auto [coeff, gcd] = gcd_extended(is[0], is[1]); 
			// Initialization of coefficients
			auto [x, y] = coeff; 
			// Returning calculated values
			return std::pair{ std::array<std::int64_t, 2>{x, y}, gcd }; 
		}
		else
		{
			// Declaration of an array of numbers
			std::array<I, size - 1> all_but_last{}; 
			// Checking how large the array must be
			for (auto [i, el] : is | std::views::take(size - 1) | std::views::enumerate) 
			{
				all_but_last[i] = el;
			}

			// Calling out the function for all but last elements of the array and passing the results to variables
			auto [coeff, gcd] = gcd_extended(all_but_last); 
			// Calling out the function for last and one before last coefficient and passing the results to variables
			auto [coeff2, gcd2] = gcd_extended(gcd, is[size - 1]); 
			// Assigning alpha and beta coefficients from results from coeff2 
			auto [alpha, beta] = coeff2; 
			// Declaration of an array of coefficients
			std::array<std::int64_t, size> x{}; 
			for (auto [ind, c] : coeff | std::views::enumerate)
			{
				// Calculating the coefficient and assigning it to an array
				x[ind] = alpha * c; 
			}
			// Assigning a beta value to the one before the last element of an array
			x[size - 1] = beta; 
			// Returning coefficients
			return std::pair{ x, gcd2 }; 
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
			std::basic_ostringstream<char> steps;
			steps << "steps of the extended euclidean algorithm:\n";
			std::array<int, sizeof...(Is)> numbers{ nums... };
			constexpr size_t N = sizeof...(Is);

			auto [coeff, gcd] = gcd_extended(numbers);
			steps << "calculating greatest common divisor: ";
			((steps << nums << ", "), ...);
			steps << "\n\n";

			int iterator = 0;
			for (size_t i = 0; i < N - 1; ++i) {
				iterator++;
				std::cout << "Step: " << iterator << "\n";
				steps << "GCD(" << numbers[i] << ", " << numbers[i + 1] << ")\n";

				// Perform the extended Euclidean algorithm for the current pair of numbers
				auto [coeff, current_gcd] = gcd_extended(numbers[i], numbers[i + 1]);

				// Extract coefficients
				auto [x, y] = coeff;

				// Display coefficients
				steps << "Coefficients (x, y) for GCD: (" << x << ", " << y << ")\n";

				// Update numbers array for the next iteration
				numbers[i + 1] = current_gcd;

				// Display the updated numbers array
				steps << "Updated elements of the array: ";
				for (size_t j = 0; j < N; ++j) {
					steps << numbers[j];
				};
			}
			return "some string to return idk yet which one";
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