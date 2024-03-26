#include <iostream>
#include <concepts>
#include <variant>
#include <expected>
#include <variant>
#include <vector>
#include <string>
#include <expected>
#include <ranges>
#include <algorithm>
#include <cstdlib>
#include <print>

namespace representation {

	template <std::integral I>
	struct fraction {
		I numerator;
		I denominator;

		std::string plus_or_minus(fraction<I>);
	};

	template <std::uint8_t base>
	struct expansion {
		std::string whole;
		std::string fractial;
		std::string period;
	};

	template <std::integral I, std::uint8_t base>
	expansion<base> expand(fraction<I>, expansion<base>);

	template <std::integral I, std::uint8_t base>
	std::string integral_number(fraction<I>, expansion<base>);

	template <std::integral I, std::uint8_t base>
	std::string fractional_number(fraction<I>, expansion<base>&);

	template <std::integral I, std::uint8_t base>
	std::vector<I> set_of_rest(std::string&, fraction<I>, I&, expansion<base>&);

	template <std::integral I, std::uint8_t base>
	std::string periodic(std::vector<I>, std::string, I, expansion<base>&);

	template <std::integral I, std::uint8_t base>
	fraction<I> dexpand(fraction<I>, expansion<base>);

	int gcd(int, int);
	
	int lcm2(int, int);

	int lcm3(int, int, int);

	template <std::integral I, std::uint8_t base>
	void n_d_whole(I&, I&, expansion<base>);

	template <std::integral I, std::uint8_t base>
	void n_d_fractial(I&, I&, expansion<base>);

	template <std::integral I, std::uint8_t base>
	void n_d_period(I&, I&, expansion<base>);

	template <std::integral I>
	void sum_fraction(I&, I&, I, I, I, I, I, I);

	template <std::integral I>
	void reduce_fraction(I&, I&);

	template<std::integral I, std::uint8_t base>
	expansion<base> expand(fraction<I> my_fraction, expansion<base> my_expansion){
		my_expansion.whole = my_fraction.plus_or_minus(my_fraction) + integral_number(my_fraction, my_expansion);
		my_expansion.fractial = fractional_number(my_fraction, my_expansion);

		return my_expansion;
	}

	template<std::integral I, std::uint8_t base>
	std::string integral_number(fraction<I> my_fraction, expansion<base> my_expansion){
		std::string result = "";
		const int ascii_table_chars_start = 55;
		I rest, integral = my_fraction.numerator / my_fraction.denominator;

		if (integral > 0) {
			while (integral > 0) {
				rest = integral % base;
				if (rest < 10) {
					result = std::to_string(rest) + result;
				}
				else {
					result = static_cast<char>(rest + ascii_table_chars_start) + result;
				}
				integral /= base;
			}
		}
		else {
			result = "0";
		}
		
		return result;
	}

	template<std::integral I, std::uint8_t base>
	std::string fractional_number(fraction<I> my_fraction, expansion<base>& my_expansion){
		std::string result = "";
		I rest = my_fraction.numerator % my_fraction.denominator;
		std::vector<I> rs = set_of_rest(result, my_fraction, rest, my_expansion);

		if (rest != 0) {
			result = periodic(rs, result, rest, my_expansion);
		}

		return result;
	}

	template<std::integral I, std::uint8_t base>
	std::vector<I> set_of_rest(std::string& result1, fraction<I> my_fraction, I& rest, expansion<base>& my_expansion){
		const int ascii_table_chars_start = 55;
		std::vector<I> rs;

		while (rest != 0 && std::find(rs.begin(), rs.end(), rest) == rs.end()) {
			rs.push_back(rest);
			my_fraction.numerator = rest * base;
			if (my_fraction.numerator / my_fraction.denominator < 10) {
				result1 += std::to_string(my_fraction.numerator / my_fraction.denominator);
			}
			else {
				result1 += static_cast<char>(my_fraction.numerator / my_fraction.denominator + ascii_table_chars_start);
			}
			rest = my_fraction.numerator % my_fraction.denominator;
		}

		return rs;;
	}

	template<std::integral I, std::uint8_t base>
	std::string periodic(std::vector<I> rs, std::string result, I rest, expansion<base>& my_expansion){
		int index = std::ranges::distance(rs.begin(), std::ranges::find(rs, rest));
		my_expansion.period = result.substr(index, result.size() - index); 
		result = result.substr(0, index);

		return result;
	}

	template<std::integral I, std::uint8_t base>
	fraction<I> dexpand(fraction<I> my_fraction, expansion<base> my_expansion){
		I numerator = 0, denominator = 1;
		I n_whole, d_whole, n_fractial, d_fractial, n_period, d_period;

		n_d_whole(n_whole, d_whole, my_expansion);
		n_d_fractial(n_fractial, d_fractial, my_expansion);
		n_d_period(n_period, d_period, my_expansion);

		sum_fraction(numerator, denominator, n_whole, n_fractial, n_period, d_whole, d_fractial, d_period);
		reduce_fraction(numerator, denominator);

		my_fraction.numerator = numerator;
		my_fraction.denominator = denominator;

		return my_fraction;
	}

	template<std::integral I, std::uint8_t base>
	void n_d_whole(I& numerator, I& denominator, expansion<base> my_expansion) {
		const int ascii_table_chars_start = 7;
		I j = my_expansion.whole.length() - 1;

		denominator = 1;
		numerator = 0;

		for (char c : my_expansion.whole) {
			if (c - '0' < 10) {
				numerator += (c - '0') * std::pow(base, j);
			}
			else {
				numerator += (c - '0' - 7) * std::pow(base, j);
			}
		}
	}

	template<std::integral I, std::uint8_t base>
	void n_d_fractial(I& numerator, I& denominator, expansion<base> my_expansion) {
		const int ascii_table_chars_start = 7;
		I j = my_expansion.fractial.length() - 1;

		denominator = std::pow(base, j + 1);
		numerator = 0;

		for (char c : my_expansion.fractial) {
			if (c - '0' < 10) {
				numerator += (c - '0') * std::pow(base, j);
			}
			else {
				numerator += (c - '0' - 7) * std::pow(base, j);
			}
		}
	}

	template<std::integral I, std::uint8_t base>
	void n_d_period(I& numerator, I& denominator, expansion<base> my_expansion) {
		const int ascii_table_chars_start = 7;
		I j = my_expansion.period.length() - 1;

		denominator = std::pow(base, my_expansion.period.length() + my_expansion.fractial.length()) - std::pow(base, my_expansion.fractial.length());
		numerator = 0;

		for (char c : my_expansion.period) {
			if (c - '0' < 10) {
				numerator += (c - '0') * std::pow(base, j);
			}
			else {
				numerator += (c - '0' - 7) * std::pow(base, j);
			}
		}
	}

	template <std::integral I>
	void sum_fraction(I& numerator, I& denominator, I n_whole, I n_fractial, I n_period, I d_whole, I d_fractial, I d_period) {
		denominator = lcm3(d_whole, d_fractial, d_period);
		I help_variable = denominator / d_whole;
		n_whole *= help_variable;
		help_variable = denominator / d_fractial;
		n_fractial *= help_variable;
		help_variable = denominator / d_period;
		n_period *= help_variable;
		numerator = n_whole + n_fractial + n_period;
	}

	template <std::integral I>
	void reduce_fraction(I& numerator, I& denominator) {
		I help_variable = gcd(numerator, denominator);
		numerator /= help_variable;
		denominator /= help_variable;
	}

	template<std::integral I>
	std::string fraction<I>::plus_or_minus(fraction<I> my_fraction){
		if ((my_fraction.numerator > 0 && my_fraction.denominator > 0) || (my_fraction.numerator < 0 && my_fraction.denominator < 0)) {
			return  "";
		}
		else {
			return "-";
		}
	}

	int gcd(int a, int b) {
		if (b == 0)
			return a;
		return gcd(b, a % b);
	}

	int lcm2(int a, int b) {
		int wynik_nwd = gcd(a, b);
		return (a * b) / wynik_nwd;
	}

	int lcm3(int a, int b, int c) {
		return lcm2(lcm2(a, b), c);
	}
}