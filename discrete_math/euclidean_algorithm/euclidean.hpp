//libraries
#include <concepts>
#include <tuple>
#include <string>
#include <algorithm>
#include <numeric>
#include <ranges>
#include <print>
#include <sstream>
#include <vector>

namespace algorithms
{
	// Structure containing Bézout's coefficients and greatest common divisor
	template <std::integral... Is>
	struct euclidean_result
	{
		std::tuple<Is...> coefficients{};
		std::uint64_t GCD{};
	};

	// Given sequence of numbers of the form i,j,k, when=
	std::string to_description(std::vector<int>results)
	{
		std::string steps;
		for (int a = 0; a < results.size(); a += 3)
		{
			auto i = results.at(a);
			auto j = results.at(a + 1);
			auto quotient = results.at(a + 2);

			steps += std::format("\n----------------------\na = {0} b = {1}\n{0} / {1}\n"
				"Dividing number a by number b, result is {2}\n\n"
				"Then we count modulo (rest from division) numbers a and b, write it as number b\n"
				"{1} = {0} % {1}\n"
				"Switching up places of a and b, which were counted above\n"
				"We calculate coefficient x by subtraction from last coefficient's x product\n"
				"Switching up places of previous coefficient x with temporary coefficient x, which were counted above\n"
				"We do the same with coefficient y\n",
				i, j, quotient);
		}
		steps += "----------------------------------------------------------------------------------------";
		return steps;
	}

	// Function calculating coefficients and gcd for 2 elements, 
	// apart from euclidean_result returns sequence of numbers produced by euclidean algorithm 
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
		return { result, results };
	}


	// Function calculating coefficients and gcd for many numbers
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

			steps += "\nwe're using this formula: gcd(a1, a2, ...) = gcd(gcd(a_1, a_2), a_3, ...)"
				"\nfirstly we start with gcd for two numbers, then using the GCD of the first two numbers and the next number"
				"\nwe calculate a new GCD, replacing the first two numbers with the result of the calculated GCD before"
				"\nprocess is repeated until we use all the given digits"
				"\nsteps of the extended euclidean algorithm:\n"
				"calculating greatest common divisor: ";
			steps += (std::to_string(nums) + ...);

			std::array<int, sizeof...(Is)> numbers{ nums... };
			constexpr size_t N = sizeof...(Is);

			auto [coeff, gcd] = gcd_extended(numbers);

			for (size_t i = 0; i < N - 1; ++i) {


				auto [result, results] = gcd_extended(numbers[i], numbers[i + 1]);
				auto [coeff, current_gcd] = result;
				auto [x, y] = coeff;

				steps += to_description(results);
				steps += std::format("\nStep: {}\nGCD({}, {})\n"
					"Coefficients (x, y) for GCD: ({}, {})"
					"\nUpdated elements of the array: ",
					i + 1, numbers[i], numbers[i + 1], x, y);

				numbers[i + 1] = current_gcd;

				for (size_t j = 0; j < N; ++j) {
					steps += std::to_string(numbers[j]) + " ";
				};
			}
			return steps;
		}
	};

	//Checks gcd(i, j, k) == expected_gcd and checks if Bézout's identity is correct
	bool test(int i, int j, int k, int64_t expected_gcd)
	{
		algorithms::euclidean e;
		auto [coeff, gcd] = e(i, j, k);
		auto [c1, c2, c3] = coeff;

		return (gcd == expected_gcd) && (c1 * i + c2 * j + c3 * k == expected_gcd);
	}
	//Checks gcd(i, j) == std::gcd(i,j) and checks if Bézout's identity is correct
	bool test(int i, int j) {
		algorithms::euclidean e;
		auto [coeff, gcd] = e(i, j);
		auto [c1, c2] = coeff;
		return (gcd == std::gcd(i, j)) && (c1 * i + c2 * j == std::gcd(i, j));
	}

	//This is an example function calculating gcd and Bézout's identity for two integers
	std::string example_gcd_for_two_numbers(int number1, int number2)
	{
		auto [result, numbers] = algorithms::gcd_extended(number1, number2);
		auto [coefficients, gcd] = result;
		auto [x, y] = coefficients;
		std::println("\n\n\nExample1:\nStarting with {0}, {1}, gcd = {2}, coefficients x = {3}, y = {4}.\n"
			"You can check if: {3} * {0} + {4} * {1} = {3}.", number1, number2, gcd, x, y);

		return algorithms::to_description(numbers);
	}

	//This is an example function calculating gcd and Bézout's identity for three integers
	std::string example_gcd_for_n_numbers(int number1, int number2, int number3)
	{
		algorithms::euclidean euclidean;

		auto [coeff, gcd2] = euclidean(number1, number2, number3);
		auto [coeff1, coeff2, coeff3] = coeff;

		std::print("\n\n\nExample2:\nStarting with {0}, {1}, {2}, coeff = {3}, {4}, {5}, gcd2 = {6}.\n"
			"You can check if: {0} * {3} + {1} * {4} + {2} * {5} = {6}.\n",
			number1, number2, number3, coeff1, coeff2, coeff3, gcd2);

		return euclidean.showSteps(number1, number2, number3);
	}
}