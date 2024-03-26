#pragma once
#include <iostream>
#include <concepts>
#include <format>
#include <vector>
#include <string>

namespace representation {
    template <std::integral I>
    struct fraction {
        I numerator{ 0 };
        I denominator{ 1 };

        /*spawdŸ sobie operator overloading na cppreferecne*/
        constexpr fraction<I>& operator-=(I i) {
            numerator -= i * denominator;
            return *this;
        }
    };

    template <std::integral I>
    constexpr fraction<I> operator+(fraction<I> f, fraction<I> g) {
        fraction<I> result{};
        result.numerator =
            f.numerator * g.denominator + g.numerator * f.denominator;
        result.denominator = f.denominator * g.denominator;
        return result;
    }

} // namespace representation

template <std::integral I>
struct std::formatter<representation::fraction<I>> {

    constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }

    auto format(const representation::fraction<I>& f,
        std::format_context& ctx) const {
        return std::format_to(ctx.out(), "{}/{}", f.numerator, f.denominator);
    }
};

namespace representation {
    template <std::uint8_t base>
    struct expansion {
        std::string whole{};
        std::string fractial{};
        std::string period{};
    };
}  // namespace representation

template <std::uint8_t base>
struct std::formatter<representation::expansion<base>> {

    constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }

    auto format(const representation::expansion<base>& exp,
        std::format_context& ctx) const {
        return std::format_to(
            ctx.out(), "{}.{}({})", exp.whole, exp.fractial, exp.period);
    }
};

namespace representation {

    template <std::uint8_t base>
    inline auto expand_whole(std::integral auto i) -> expansion<base> {
        expansion<base> expan{};
        const std::size_t max_integer_string_length{ 129 };
        auto& whole = expan.whole;
        whole.resize(max_integer_string_length); /*rezerwacja miejsca*/
        auto result =
            std::to_chars(whole.data(), whole.data() + whole.size(), i, base);
        /*co jest w result - za pomoc¹ tego zmodyfikowaæ whole*/
        /*skorzystaj z result.ptr*/
        const std::size_t used_bytes{ /*ile dana liczba zajê³a miejsca*/ };
        whole.resize(used_bytes);

        return expan;
    }
    // przyk³ad representation::expand<2>(1), sprawdŸ czy dosta³eœ string "1"
    // przyk³ad representation::expand<10>(1111), sprawdŸ czy dosta³eœ string
    // "1111"

    template <std::uint8_t base, std::integral I>
    inline auto expand_period_part(expansion<base>& expan, fraction<I> frac, I rest, std::vector<I> rs, std::string result)
        -> void {
        int index = std::ranges::distance(rs.begin(), std::ranges::find(rs, rest));
        expan.period = result.substr(index, result.size() - index);
        expan.fractial = result.substr(0, index);
    }

    template <std::uint8_t base, std::integral I>
    inline auto expand_fractional_part(expansion<base>& expan, fraction<I> frac)
        -> void {
        const int ascii_table_chars_start = 55;
        std::string result = "";
        I rest = frac.numerator% frac.denominator;
        auto num = frac.numerator;
        auto den = frac.denominator;

        std::vector<I> rs;

        while (rest != 0 && std::find(rs.begin(), rs.end(), rest) == rs.end()) {
            rs.push_back(rest);
            frac.numerator = rest * base;
            if (frac.numerator / frac.denominator < 10) {
                result += std::to_string(frac.numerator / frac.denominator);
            }
            else {
                result += static_cast<char>(frac.numerator / frac.denominator + ascii_table_chars_start);
            }
            rest = frac.numerator % frac.denominator;
        }

        if (rest != 0) {
            expand_period_part(expan, frac, rest, rs, result);
        }
        else {
            expan.fractial = result;
            expan.period = "";
        }
        
    }

    template <std::uint8_t base, std::integral I>
    inline auto expand(fraction<I> frac) -> expansion<base> {
        expansion<base> expan{};
        auto whole = frac.numerator / frac.denominator;
        expan = expand_whole<base>(whole);

        frac -= whole;
        
        expand_fractional_part(expan ,frac);

        return expan;
    }
    template <std::uint8_t base, std::integral I>
    inline I dexpand_h(std::string whole) {
        I integer{};
        auto result = std::from_chars(
            whole.data(), whole.data() + whole.size(), integer, base);
        return integer;
    }


    inline auto pow(std::integral auto i, std::integral auto exponent) {
        return std::pow(i, exponent);
    }

    template <std::uint8_t base, std::integral I>
    inline auto dexpand(expansion<base> expan) -> fraction<I> {
        /*masz liczbê postaci whole.fraction(period)*/
        fraction<I> whole{ .numerator{0}, .denominator{1} };
        whole.numerator = dexpand_h<base, I>(expan.whole);
        fraction<I> fractional{};
        fractional.numerator = dexpand_h<base, I>(expan.fractial);
        fractional.denominator = dexpand_h<base, I>(
            std::string("1") + std::string(expan.fractial.size(), '0')); 

        fraction<I> period{};
        period.numerator = dexpand_h<base, I>(expan.period);
        period.denominator = pow(base, expan.fractial.size() + expan.period.size()) - pow(base, expan.period.size());

        return whole + fractional + period;
    }

}  // namespace representation