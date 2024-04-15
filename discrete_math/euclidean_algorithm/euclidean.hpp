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
#include <vector>

namespace algorithms
{
	// Structure containing coefficients and greatest common divisor
	template <std::integral... Is>
	struct euclidean_result
	{
		std::tuple<Is...> coefficients{};
		std::uint64_t GCD{};
	};

	std::string to_description(std::vector<int>results)
	{
		std::string steps;
		for (int a = 0; a < results.size(); a += 3)
		{
			auto i = results.at(a);
			auto j = results.at(a + 1);
			auto quotient = results.at(a + 2);

			steps += std::format("\n----------------------\na = {} b = {}\n{} / {}\n", i, j, i, j);
			steps += std::format("Dividing number a by number b, result is {}\n\n", quotient);
			steps += "Then we count modulo (rest from division) numbers a and b, write it as number b\n";
			steps += std::format("{} = {} % {}\n", j, i, j);
			steps += "Switching up places of a and b, which were counted above\n"
				"We calculate coefficient x by subtraction from last coefficient's x product\n";
			"Switching up places of previous coefficient x with temporary coefficient x, which were counted above\n"
				"We do the same with coefficient y\n";
		}
		steps += "----------------------------------------------------------------------------------------";
		return steps;
	}

	// Function calculating coefficients and gcd for 2 elements
	template <std::integral I, std::integral J>
	constexpr auto gcd_extended(I i, J j) -> std::pair<euclidean_result<I, J>, std::vector<int>>
	{
		euclidean_result<I, J> result{};
		int x = 0, y = 1, x_prev = 1, y_prev = 0;
		std::vector<int>results;
		while (j != 0)
		{
			int quotient = i / j;
			i = std::exchange(j, i % j);
			x_prev = std::exchange(x, x_prev - quotient * x);
			y_prev = std::exchange(y, y_prev - quotient * y);

			results.push_back(i);
			results.push_back(j);
			results.push_back(quotient);
		}

		result.coefficients = std::tuple{ x_prev, y_prev };
		result.GCD = i;
		return { result, results }; //steps
	}


	// Function calculating coefficients and gcd for many elements
	template <std::integral I, std::size_t size>
		requires (size >= 2)
	constexpr auto gcd_extended(std::array<I, size> is) -> std::pair <std::array<std::int64_t, size>, std::uint64_t>
	{
		if constexpr (size == 2)
		{
			auto [result, step] = algorithms::gcd_extended(is[0], is[1]);
			auto [coeff, gcd] = result;
			auto [x, y] = coeff;
			return std::pair{ std::array<std::int64_t, 2>{x, y}, gcd };
		}
		else
		{
			std::array<I, size - 1> all_but_last{};
			for (auto [i, el] : is | std::views::take(size - 1) | std::views::enumerate)
			{
				all_but_last[i] = el;
			}
			auto [coeff, gcd] = algorithms::gcd_extended(all_but_last);
			auto [result2, step2] = algorithms::gcd_extended(gcd, is[size - 1]);
			auto [coeff2, gcd2] = result2;
			auto [alpha, beta] = coeff2;

			std::array<std::int64_t, size> x{};
			static_assert(std::ranges::range<decltype(coeff)>);
			for (auto [ind, c] : coeff | std::views::enumerate)
			{
				x[ind] = alpha * c;
			}
			x[size - 1] = beta;
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

			std::string steps{};

			steps += "\nwe're using this formula: gcd(a1, a2, ...) = gcd(gcd(a_1, a_2), a_3, ...)";
			steps += "\nfirstly we start with gcd for two numbers, then using the GCD of the first two numbers and the next number";
			steps += "\nwe calculate a new GCD, replacing the first two numbers with the result of the calculated GCD before;";
			steps += "\nprocess is repeated until we use all the given digits";

			steps += "\nsteps of the extended euclidean algorithm:\n";
			std::array<int, sizeof...(Is)> numbers{ nums... };
			constexpr size_t N = sizeof...(Is);

			auto [coeff, gcd] = gcd_extended(numbers);
			steps += "calculating greatest common divisor: ";
			steps += (std::to_string(nums) + ...);
			steps += "\n\n";

			for (size_t i = 0; i < N - 1; ++i) {

				steps += std::format("\nStep: {}\n", i + 1);
				steps += std::format("GCD({}, {})", numbers[i], numbers[i + 1]);

				auto [result, results] = gcd_extended(numbers[i], numbers[i + 1]);
				auto [coeff, current_gcd] = result;
				steps += to_description(results);

				auto [x, y] = coeff;

				steps += std::format("\nCoefficients (x, y) for GCD: ({}, {})", x, y);

				numbers[i + 1] = current_gcd;

				steps += "\nUpdated elements of the array: ";
				for (size_t j = 0; j < N; ++j) {
					steps += std::to_string(numbers[j]);
					steps += " ";
				};
			}
			return steps;
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