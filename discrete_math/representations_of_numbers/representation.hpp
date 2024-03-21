#include <iostream>
#include <concepts>
#include <variant>
#include <expected>
#include <variant>
#include <vector>
#include <string>

namespace representation_namespace {

    const int NUMBERS = 48;
    const int CHARS = 55;

    template <std::integral I>
    struct fraction {
        I numerator{ 0 };
        I denominator{ 1 };

        fraction(I num = 0, I den = 1) : numerator(num), denominator(den) {}
    };

    template <std::uint8_t base>
    struct expansion {
        static constexpr std::uint8_t form = base;
    };

    class representation {
    public:

        template <std::integral I>
        fraction<I> create_fraction(I, I);

        template <std::uint8_t base>
        expansion<base> create_expansion(int);

        template <std::integral I>
        std::string plus_or_minus(fraction<I>);

        template <std::integral I, std::uint8_t base>
        std::string integral_number(I, I, expansion<base>);

        template <std::integral I, std::uint8_t base>
        std::string expansion_in_base(fraction<I>, expansion<base>);

        template <std::integral I>
        std::vector<I> set_of_rest(std::string&, I, I, I, int);

        template <std::integral I>
        std::string periodic(std::vector<I>, std::string, I);

        template <std::integral I, std::uint8_t base>
        std::string fractional_number(I, I, expansion<base>);
    };

    template <std::integral I>
    fraction<I> representation::create_fraction(I numerator, I denominator) {
        return fraction<I>(numerator, denominator);
    }

    template <std::uint8_t base>
    expansion<base> representation::create_expansion(int) {
        return expansion<base>();
    }

    template <std::integral I>
    std::string representation::plus_or_minus(fraction<I> my_fraction) {
        if ((my_fraction.numerator > 0 && my_fraction.denominator > 0) || (my_fraction.numerator < 0 && my_fraction.denominator < 0)) {
            return " ";
        }
        else {
            return "-";
        }
    }

    template <std::integral I, std::uint8_t base>
    std::string representation::integral_number(I numerator, I denominator, expansion<base> my_expansion) {
        std::string result;
        I rest, integral = numerator / denominator;

        while (integral > 0) {
            rest = integral % my_expansion.form;
            if (rest < 10) {
                result = (char)(rest + NUMBERS) + result;
            }
            else {
                result = (char)(rest + CHARS) + result;
            }
            integral /= my_expansion.form;
        }

        return result;
    }

    template <std::integral I>
    std::vector<I> representation::set_of_rest(std::string& result, I numerator, I denominator, I rest, int base) {
        std::vector<I> rs;

        while (rest != 0 && std::find(rs.begin(), rs.end(), rest) == rs.end()) {
            rs.push_back(rest);
            numerator = rest * base;
            if (numerator / denominator < 10) {
                result += (char)(numerator / denominator + NUMBERS);
            }
            else {
                result += (char)(numerator / denominator + CHARS);
            }
            rest = numerator % denominator;
        }

        return rs;
    }

    template <std::integral I>
    std::string representation::periodic(std::vector<I> rs, std::string result, I rest) {
        auto a = std::find(rs.begin(), rs.end(), rest);
        int index = std::distance(rs.begin(), a);
        result.insert(index + result.find('.') + 1, "(");
        result += ')';

        return result;
    }

    template <std::integral I, std::uint8_t base>
    std::string representation::fractional_number(I numerator, I denominator, expansion<base> my_expansion) {
        std::string result = ".";
        I rest = numerator % denominator;

        std::vector<I> rs = set_of_rest(result, numerator, denominator, rest, base);
        if (rest != 0) {
            result = periodic(rs, result, rest);
        }

        return result;
    }

    template <std::integral I, std::uint8_t base>
    std::string representation::expansion_in_base(fraction<I> f, expansion<base> e) {
        std::string result = plus_or_minus(f);
        result += integral_number(std::abs(f.numerator), std::abs(f.denominator), e);
        if (f.numerator % f.denominator != 0) {
            result += fractional_number(std::abs(f.numerator), std::abs(f.denominator), e);
        }

        return result;
    }

}
