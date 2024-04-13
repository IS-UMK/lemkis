#pragma once
#include <iostream>
#include <numeric>
#include <cmath>
#include <charconv>
#include <algorithm>
#include <ranges>
#include <concepts>
#include <format>
#include <vector>
#include <string>
#include <print>

namespace representation {

	/*
		description:
			defines the fraction structure to represent
			a fraction with integer numerator and denominator.
		members:
			I numerator - integer numerator of the fraction
			I denominator - integer denominator of the fraction
		methods:
			operator-= - subtracts an integer value from the fraction
	*/
	template <std::integral I>
	struct fraction {
		I numerator{ 0 };
		I denominator{ 1 };

		constexpr fraction<I>& operator-=(I i) {
			numerator -= i * denominator;
			return *this;
		}
		/*
		description:
			reduce fraction
	*/
		void reduce() {
			I divider = std::gcd(numerator, denominator);
			numerator /= divider;
			denominator /= divider;
		}

	};



	/*
		description:
			overloads the addition operator for fractions,
			allowing addition of fractions
	*/
	template <std::integral I>
	constexpr fraction<I> operator+(fraction<I> f, fraction<I> g) {
		fraction<I> result{};
		result.numerator =
			f.numerator * g.denominator + g.numerator * f.denominator;
		result.denominator = f.denominator * g.denominator;
		result.reduce();
		return result;
	}

} // namespace representation

	/*
		description:
			enables formatting of output data
			for representation::fraction<I>
	*/
template <std::integral I>
struct std::formatter<representation::fraction<I>> {

	template <typename FormatParseContext>
	constexpr auto parse(FormatParseContext& ctx) { return ctx.begin(); }


	template <typename FormatContext>
	auto format(const representation::fraction<I>& f,
		FormatContext& ctx) const {
		return std::format_to(ctx.out(), "{}/{}", f.numerator, f.denominator);
	}
};

/*
	description:
		defines the expansion structure to represent
		the expansion of a fraction in a specified base.
		whole.fractial(period)
	members:
		std::string whole - string representing 
			the whole part of the expansion.
		std::stringfractial - string representing
			the non-repeating fractional part of the expansion.
		std::stringperiod - string representing
			the repeating period of the expansion.
*/
namespace representation {
	template <std::uint8_t base>
	struct expansion {
		std::string whole{};
		std::string fractial{};
		std::string period{};
	};
}  // namespace representation

	/*
		description:
			enables formatting of output data 
			for representation::expansion<base>
	*/
template <std::uint8_t base>
struct std::formatter<representation::expansion<base>> {

	template <typename FormatParseContext>
	constexpr auto parse(FormatParseContext& ctx) { return ctx.begin(); }



	template <typename ParseContext>
	auto format(const representation::expansion<base>& exp,
		ParseContext& ctx) const {
		return std::format_to(
			ctx.out(), "{}.{}({})", exp.whole, exp.fractial, exp.period);
	}
};

namespace representation {

	/*
		description:
			expands the whole part of a fraction in the specified base
	*/
	template <std::uint8_t base>
	inline auto expand_whole(std::integral auto i) -> expansion<base> {
		expansion<base> expan{};

		const std::size_t max_integer_string_length{ 129 };
		auto& whole = expan.whole;
		whole.resize(max_integer_string_length);
		auto result =
			std::to_chars(whole.data(), whole.data()
				+ whole.size(), i, base);

		const std::size_t used_bytes{ static_cast<std::size_t>(result.ptr
			- whole.data()) };
		whole.resize(used_bytes);

		expan.whole = whole;
		return expan;
	}

	/*
		description:
			expands the repeating part of a fraction in the specified base
	*/
	template <std::uint8_t base, std::integral I>
	inline auto expand_period_part(expansion<base>& expan, fraction<I> frac,
		I remainder, std::vector<I> rs, std::string result)
		-> void {
		int index = std::ranges::distance(rs.begin(), 
			std::ranges::find(rs, remainder));
		expan.period = result.substr(index, result.size() - index);
		expan.fractial = result.substr(0, index);
	}

	/*
		 description:
			 expands the fractional part of a fraction in the specified base
	*/
	template <std::uint8_t base, std::integral I>
	inline auto expand_fractional_part(expansion<base>& expan,
		fraction<I> frac)
		-> void {
		const int ascii_table_chars_start = 55;
		std::string result = "";
		I remainder = frac.numerator % frac.denominator;
		auto num = frac.numerator;
		auto den = frac.denominator;
		std::vector<I> rs;
		while (remainder != 0 && std::find(rs.begin(), rs.end(), remainder) 
			== rs.end()) {
			rs.push_back(remainder);
			frac.numerator = remainder * base;
			if (frac.numerator / frac.denominator < 10)
				result += std::to_string(frac.numerator / frac.denominator);
			else
				result += static_cast<char>(frac.numerator / frac.denominator
					+ ascii_table_chars_start);
			remainder = frac.numerator % frac.denominator;
		}

		if (remainder != 0)
			expand_period_part(expan, frac, remainder, rs, result);
		else {
			expan.fractial = result;
			expan.period = "";
		}
	}

	/*
		description:
			combines functions (expand_whole,
			expand_period_part, expandfractional_part)
			to expand a fraction into its whole, fractional, 
			and repeating parts
	*/
	template <std::uint8_t base, std::integral I>
	inline auto expand(fraction<I> frac) -> expansion<base> {
		expansion<base> expan{};
		auto whole = frac.numerator / frac.denominator;
		expan = expand_whole<base>(whole);
		frac -= whole;
		expand_fractional_part(expan, frac);
		return expan;
	}

	/*
		description:
			converts a string representing a number in
			a given base to its integral representation
	*/
	template <std::uint8_t base, std::integral I>
	inline I dexpand_h(std::string whole) {
		I integer{};
		auto result = std::from_chars(
			whole.data(), whole.data() + whole.size(), integer, base);
		return integer;
	}

	/*
		description:
			calculates the power of an integer
	*/
	inline auto pow(std::integral auto i, std::integral auto exponent) {
		if (exponent == 0)
			return 1;
		else if (exponent % 2 == 0)
			return pow(i * i, exponent / 2);
		else
			return i * pow(i * i, (exponent - 1) / 2);
	}

	/*
		description:
			reconstructs the whole part of a fraction from its expansion
	*/
	template <std::uint8_t base, std::integral I>
	inline auto dexpand_whole_part(expansion<base> exp) -> fraction<I> {
		fraction<I> whole{ .numerator{0}, .denominator{1} };
		whole.numerator = dexpand_h<base, I>(exp.whole);
		return whole;
	}

	/*
		description:
			reconstructs the non-repeating fractional part of
			a fraction from its expansion
	*/
	template <std::uint8_t base, std::integral I>
	inline auto dexpand_fractial_part(expansion<base> exp) -> fraction<I> {
		fraction<I> fractional{};
		if (!exp.fractial.empty()) {
			fractional.numerator = dexpand_h<base, I>(exp.fractial);
			fractional.denominator = dexpand_h<base, I>(
				std::string("1") + std::string(exp.fractial.size(), '0'));
		}
		else {
			fractional.numerator = 0;
			fractional.denominator = 1;
		}
		return fractional;
	}

	/*
		description:
			reconstructs the repeating period of
			a fraction from its expansion
	*/
	template <std::uint8_t base, std::integral I>
	inline auto dexpand_period_part(expansion<base> exp) -> fraction<I> {
		fraction<I> period{};
		period.numerator = dexpand_h<base, I>(exp.period);
		if (!exp.fractial.empty()) {
			period.denominator = pow(base, exp.fractial.size()
				+ exp.period.size()) - pow(base, exp.period.size());
		}
		else {
			period.denominator = pow(base, exp.period.size()) - 1;
		}
		return period;
	}

	/*
		description:
			combines functions (dexpand_period_part,
			dexpand_fractial_part, dexpand_whole_part)
			to reconstruct a fraction from its expanded representation.
	*/
	template <std::uint8_t base, std::integral I>
	inline auto dexpand(expansion<base> expan) -> fraction<I> {
		fraction<I> whole = dexpand_whole_part<base, I>(expan);
		fraction<I> fractional = dexpand_fractial_part<base, I>(expan);
		fraction<I> period = dexpand_period_part<base, I>(expan);
		return whole + fractional + period;
	}
}  // namespace representation

template <std::uint8_t base>
void mutual_inverses(representation::expansion<base>& expansion,
	representation::fraction<int> original_fraction) {
	representation::fraction<int> my_fraction
		= representation::dexpand<base, int>(expansion);
	std::print("\nMutual inverse: {}\n", my_fraction);
}

template <std::uint8_t base>
void mutual_inverses(representation::fraction<int>& original_fraction,
	representation::expansion<base> original_expansion) {
	representation::expansion<base> my_expansion
		= representation::expand<base>(original_fraction);
	std::print("\nMutual inverse: {}\n", my_expansion);
}

template <std::uint8_t base>
void print_tests(representation::fraction<int>& fraction,
	representation::expansion<base>& expansion_1,
	representation::expansion<base>& expansion_2) {
	std::print("\nInput: {}\nBase: {}\nWhat we want to get: {},\nOutput: {}",
		fraction, base, expansion_1, expansion_2);
	mutual_inverses(expansion_2, fraction);
}

template <std::uint8_t base>
void print_tests(representation::expansion<base>& expansion,
	representation::fraction<int>& fraction_1,
	representation::fraction<int>& fraction_2) {
	std::print("\nInput: {}\nBase: {}\nWhat we want to get: {},\nOutput: {}",
		expansion, base, fraction_1, fraction_2);
	mutual_inverses(fraction_2, expansion);
}

template <std::uint8_t base>
void representation_test(int numerator, int denominator,
	std::string whole, std::string fractial, std::string period) {
	representation::fraction<int> original_fraction{ numerator, denominator };
	representation::expansion<base> expected_expansion{ 
		whole, fractial, period };
	representation::expansion<base> expansion_result
		= representation::expand<base>(original_fraction);
	print_tests(original_fraction, expected_expansion, expansion_result);
}

template <std::uint8_t base>
void representation_test(std::string whole, std::string fractial,
	std::string period, int numerator, int denominator) {
	representation::expansion<base> original_expansion{ 
		whole, fractial, period };
	representation::fraction<int> expected_fraction{ numerator, denominator };
	representation::fraction<int> fraction_result
		= representation::dexpand<base, int>(original_expansion);
	print_tests(original_expansion, expected_fraction, fraction_result);
}

void representation_tests() {
	using namespace representation;
	const int decimal = 10;
	const int binary = 2;
	const int octal = 8;
	const int hexal = 16;

	representation_test<decimal>(64, 30, "2", "1", "3");
	representation_test<binary>(3, 2, "1", "1", "0");
	representation_test<octal>(75, 56, "1", "2", "3");
	representation_test<hexal>(91, 80, "1", "2", "3");
	representation_test<decimal>("1", "0", "1", 91, 90);
	representation_test<binary>("1", "00", "01", 13, 12);
	representation_test<octal>("10", "01", "12", 32329, 4032);
	representation_test<hexal>("10", "ca", "0", 2149, 128);
}