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

namespace representation{

    template <std::integral I>
    struct fraction {
        I numerator{ 0 };
        I denominator{ 1 };
        std::string plus_or_minus(I, I);
    };

    template <std::uint8_t base>
    struct expansion {
        std::uint8_t base_v = base;
        std::string whole;
        std::string fractial;
        std::string period;
    };

    template <std::integral I, std::uint8_t base>
    std::string integral_number(I, I, expansion<base>&);

    template <std::integral I, std::uint8_t base>
    void expande(fraction<I>&, expansion<base>&);

    template <std::integral I>
    std::vector<I> set_of_rest(std::string&, I, I, I&, int);

    template <std::integral I, std::uint8_t base>
    std::string periodic(std::vector<I>, std::string, I, expansion<base>&);

    template <std::integral I, std::uint8_t base>
    std::string fractional_number(I, I, expansion<base>&);

    template <std::integral I, std::uint8_t base>
    std::string integral_number(I numerator, I denominator, expansion<base>& my_expansion) {
        const int ascii_table_numbers_start = 48;
        const int ascii_table_chars_start = 55;
        std::string result;
        I rest, integral = numerator / denominator;

        while (integral > 0) {
            rest = integral % my_expansion.base_v;
            if (rest < 10) {
                result = std::to_string(rest) + result;
            }
            else {
                result = static_cast<char>(rest + ascii_table_chars_start) + result;
            }
            integral /= my_expansion.base_v;


        }

        return result;
    }

    template <std::integral I>
    std::vector<I> set_of_rest(std::string& result, I numerator, I denominator, I& rest, int base) {
        std::vector<I> rs;
        const int ascii_table_numbers_start = 48;
        const int ascii_table_chars_start = 55;

        while (rest != 0 && std::find(rs.begin(), rs.end(), rest) == rs.end()) {
            rs.push_back(rest);
            numerator = rest * base;
            if (numerator / denominator < 10) {
                result += std::to_string(numerator / denominator);
            }
            else {
                result += static_cast<char>(numerator / denominator + ascii_table_chars_start);
            }
            rest = numerator % denominator;
        }

        return rs;
    }

    template <std::integral I, std::uint8_t base>
    std::string periodic(std::vector<I> rs, std::string result, I rest, expansion<base>& my_expansion) {
        int index = std::ranges::distance(rs.begin(), std::ranges::find(rs, rest));
        my_expansion.fractial = result.substr(0, index);
        result.insert(index, "(");
        result += ')';
        my_expansion.period = result.substr(index + 1, result.size() - index - 2);

        return result;
    }

    template <std::integral I, std::uint8_t base>
    std::string fractional_number(I numerator, I denominator, expansion<base>& my_expansion) {
        std::string result;
        I rest = numerator % denominator;

        std::vector<I> rs = set_of_rest(result, numerator, denominator, rest, my_expansion.base_v);
        if (rest != 0) {
            result = periodic(rs, result, rest, my_expansion);
        }

        return result;
    }

    template <std::integral I, std::uint8_t base>
    void expand(fraction<I>& f, expansion<base>& my_expansion) {
        std::string result = f.plus_or_minus(f.numerator, f.denominator);
        result = integral_number(std::abs(f.numerator), std::abs(f.denominator), my_expansion);
        my_expansion.whole = result;
        if (f.numerator % f.denominator != 0) {
            result += ".";
            result += fractional_number(std::abs(f.numerator), std::abs(f.denominator), my_expansion);
        }
    }

    template<std::integral I>
    std::string fraction<I>::plus_or_minus(I numerator, I denominat){
        if ((numerator > 0 && denominator > 0) || (numerator < 0 && denominator < 0)) {
            return " ";
        }
        else {
            return "-";
        }
    }
}
