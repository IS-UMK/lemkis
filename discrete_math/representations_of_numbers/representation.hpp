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

	template <std::integral I, std::uint8_t base>
	void dexpand2(I&, I&, expansion<base>);

	template <std::integral I>
	void dexpand3(I&, I&, I, I, I, I, I, I);

	template <std::integral I>
	void dexpand4(I& ,I&, I , I, I, I);


	int gcd(int, int);
	
	int lcm2(int, int);

	int lcm3(int, int, int);

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

		dexpand2(numerator, denominator, my_expansion);

		my_fraction.numerator = numerator;
		my_fraction.denominator = denominator;

		return my_fraction;
	}

	template <std::integral I, std::uint8_t base>
	void dexpand2(I& numerator, I& denominator, expansion<base> my_expansion) {
		I numerator1 = std::stoi(my_expansion.whole);
		I denominator1 = 1;
		I size2 = my_expansion.fractial.length();
		I numerator2 = std::stoi(my_expansion.fractial);
		I denominator2 = std::pow(base, size2);
		I size3 = my_expansion.period.length();
		I numerator3 = std::stoi(my_expansion.period);
		I denominator3 = std::pow(base, size3 + size2) - denominator2;

		dexpand3(numerator, denominator, numerator1, numerator2, numerator3, denominator1, denominator2, denominator3);
	}

	template <std::integral I>
	void dexpand3(I& numerator, I& denominator, I numerator1, I numerator2, I numerator3, I denominator1, I denominator2, I denominator3) {
		I help_variable = lcm3(denominator1, denominator2, denominator3);
		I help_variable2 = help_variable / denominator1;
		denominator1 *= help_variable2;
		numerator1 *= help_variable2;
		help_variable2 = help_variable / denominator2;
		denominator2 *= help_variable2;
		numerator2 *= help_variable2;
		help_variable2 = help_variable / denominator3;
		denominator3 *= help_variable2;
		numerator3 *= help_variable2;
		
		dexpand4(numerator, denominator, numerator1, numerator2, numerator3, help_variable);
	}

	template <std::integral I>
	void dexpand4(I& numerator, I& denominator, I numerator1, I numerator2, I numerator3, I help_variable) {
		I dzielnik;
		denominator = help_variable;
		numerator = numerator1 + numerator2 + numerator3;
		dzielnik = gcd(numerator, denominator);
		numerator /= dzielnik;
		denominator /= dzielnik;
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