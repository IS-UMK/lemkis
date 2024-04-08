#pragma once
#include <iostream>
#include <concepts>
#include <format>
#include <vector>
#include <string>
#include <print>

namespace representation {

    /*
        opis: 
            definiuje strukturê fraction do reprezentacji u³amka z ca³kowitym licznikiem i mianownikiem.
        sk³adowe: 
            I numerator - ca³kowity licznik u³amka
            I denominator - ca³kowity mianownik u³amka
        metody:
            operator-= - odejmuje wartoœæ ca³kowit¹ od u³amka
    */
    template <std::integral I>
    struct fraction {
        I numerator{ 0 };
        I denominator{ 1 };

        constexpr fraction<I>& operator-=(I i) {
            numerator -= i * denominator;
            return *this;
        }

    };

    /*
        opis:
            przeci¹¿a operator dodawania dla u³amków, umo¿liwiaj¹c dodawanie u³amków w czasie kompilacji
            skraca u³amek wynikowy
    */
    template <std::integral I>
    constexpr fraction<I> operator+(fraction<I> f, fraction<I> g) {
        fraction<I> result{};
        result.numerator =
            f.numerator * g.denominator + g.numerator * f.denominator;
        result.denominator = f.denominator * g.denominator;
        I divider = std::_Gcd(result.numerator, result.denominator);
        result.numerator /= divider;
        result.denominator /= divider;
        return result;
    }

} // namespace representation

    /*
        opis:
            umo¿liwia formatowania danych wyjœciowych dla representation::fraction<I>
    */
template <std::integral I>
struct std::formatter<representation::fraction<I>> {

    constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }

    auto format(const representation::fraction<I>& f,
        std::format_context& ctx) const {
        return std::format_to(ctx.out(), "{}/{}", f.numerator, f.denominator);
    }
};

    /*
        opis:
            definiuje strukturê expansion do reprezentacji rozwiniêcia u³amka w okreœlonej podstawie. 
            whole.fractial(period)
        sk³adowe:
            std::string whole - ci¹g znaków reprezentuj¹cy czêœæ ca³kowit¹ rozwiniêcia. 
            std::stringfractial - ci¹g znaków reprezentuj¹cy nieokresow¹ czêœæ u³amkow¹ rozwiniêcia.
            std::stringperiod - ci¹g znaków reprezentuj¹cy okresow¹ czêœæ rozwiniêcia.
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
        opis:
            umo¿liwia formatowania danych wyjœciowych dla representation::expansion<base>
    */
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

    /*
        opis:
            rozwija czêœæ ca³kowit¹ u³amka w okreœlonej podstawie
    */
    template <std::uint8_t base>
    inline auto expand_whole(std::integral auto i) -> expansion<base> {
        expansion<base> expan{};

        const std::size_t max_integer_string_length{ 129 };
        auto& whole = expan.whole;
        whole.resize(max_integer_string_length);
        auto result =
            std::to_chars(whole.data(), whole.data() + whole.size(), i, base);

        const std::size_t used_bytes{ static_cast<std::size_t>(result.ptr - whole.data()) };
        whole.resize(used_bytes);

        expan.whole = whole;
        return expan;
    }

    /*
        opis:
            rozwija czêœæ okresow¹ u³amka w okreœlonej podstawie
    */
    template <std::uint8_t base, std::integral I>
    inline auto expand_period_part(expansion<base>& expan, fraction<I> frac, I rest, std::vector<I> rs, std::string result)
        -> void {
        int index = std::ranges::distance(rs.begin(), std::ranges::find(rs, rest));
        expan.period = result.substr(index, result.size() - index);
        expan.fractial = result.substr(0, index);
    }

    /*
        opis:
            rozwija czêœæ u³amkow¹ u³amka w okreœlonej podstawie
    */
    template <std::uint8_t base, std::integral I>
    inline auto expand_fractional_part(expansion<base>& expan, fraction<I> frac)
        -> void {
        const int ascii_table_chars_start = 55;
        std::string result = "";
        I rest = frac.numerator % frac.denominator;
        auto num = frac.numerator;
        auto den = frac.denominator;
        std::vector<I> rs;
        while (rest != 0 && std::find(rs.begin(), rs.end(), rest) == rs.end()) {
            rs.push_back(rest);
            frac.numerator = rest * base;
            if (frac.numerator / frac.denominator < 10)
                result += std::to_string(frac.numerator / frac.denominator);
            else
                result += static_cast<char>(frac.numerator / frac.denominator + ascii_table_chars_start);
            rest = frac.numerator % frac.denominator;
        }

        if (rest != 0)
            expand_period_part(expan, frac, rest, rs, result);
        else {
            expan.fractial = result;
            expan.period = "";
        }
    }

    /*
        opis:
            ³¹czy funkcje (expand_whole, expand_period_part, expandfractional_part) 
            w celu rozwiniêcia u³amka na czêœæ ca³kowit¹, u³amkow¹ i okresow¹ 
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
        opis:
            konwertuje ci¹g znaków reprezentuj¹cy liczbê w danej podstawie do jej reprezentacji ca³kowitej 
    */
    template <std::uint8_t base, std::integral I>
    inline I dexpand_h(std::string whole) {
        I integer{};
        auto result = std::from_chars(
            whole.data(), whole.data() + whole.size(), integer, base);
        return integer;
    }

    /*
        opis:
            oblicza potêgê liczby ca³kowitej
    */
    inline auto pow(std::integral auto i, std::integral auto exponent) {
        return std::pow(i, exponent);
    }

    /*
        opis:
            odtwarza czêœæ ca³kowit¹ u³amka z jego rozwiniêcia
    */
    template <std::uint8_t base, std::integral I>
    inline auto dexpand_whole_part(expansion<base> exp) -> fraction<I> {
        fraction<I> whole{ .numerator{0}, .denominator{1} };
        whole.numerator = dexpand_h<base, I>(exp.whole);
        return whole;
    }

    /*
        opis:
            odtwarza nieokresow¹ czêœæ u³amka z jego rozwiniêcia
    */
    template <std::uint8_t base, std::integral I>
    inline auto dexpand_fractial_part(expansion<base> exp) -> fraction<I> {
        fraction<I> fractional{};
        fractional.numerator = dexpand_h<base, I>(exp.fractial);
        fractional.denominator = dexpand_h<base, I>(
            std::string("1") + std::string(exp.fractial.size(), '0'));
        return fractional;
    }

    /*
        opis:
            O
            odtwarza okresow¹ czêœæ u³amka z jego rozwiniêcia
    */
    template <std::uint8_t base, std::integral I>
    inline auto dexpand_period_part(expansion<base> exp) -> fraction<I> {
        fraction<I> period{};
        period.numerator = dexpand_h<base, I>(exp.period);
        period.denominator = pow(base, exp.fractial.size() + exp.period.size()) - pow(base, exp.period.size());
        return period;
    }

    /*
        opis:
            ³¹czy funkcje (dexpand_period_part, dexpand_fractial_part, dexpand_whole_part) w celu odtworzenia u³amka z jego rozwiniêtej reprezentacji.
    */
    template <std::uint8_t base, std::integral I>
    inline auto dexpand(expansion<base> expan) -> fraction<I> {
        fraction<I> whole = dexpand_whole_part<base, I>(expan);
        fraction<I> fractional = dexpand_fractial_part<base, I>(expan);;
        fraction<I> period = dexpand_period_part<base, I>(expan);
        return whole + fractional + period;
    }
}  // namespace representation

void representation_tests() {
    using namespace representation;

    representation::fraction<int> fraction_1{ 64, 30 };
    representation::expansion<10> e_1_result{ "2", "1", "3" };
    representation::expansion<10> expansion_1 = representation::expand<10>(fraction_1);
    std::print("\nDane wejsciowe: {},\nCo chcemy otrzymac: {},\nCo otrzymujemy {}\n", fraction_1, e_1_result, expansion_1);

    representation::fraction<int> fraction_3{ 64, 30 };
    representation::expansion<2> e_3_result{ "10", "", "0010" };
    representation::expansion<2> expansion_3 = representation::expand<2>(fraction_3);
    std::print("\nDane wejsciowe: {},\nCo chcemy otrzymac: {},\nCo otrzymujemy {}\n", fraction_3, e_3_result, expansion_3);

    representation::fraction<int> fraction_5{ 64, 30 };
    representation::expansion<8> e_5_result{ "2", "", "1042" };
    representation::expansion<8> expansion_5 = representation::expand<8>(fraction_3);
    std::print("\nDane wejsciowe: {},\nCo chcemy otrzymac: {},\nCo otrzymujemy {}\n", fraction_5, e_5_result, expansion_5);

    representation::fraction<int> fraction_7{ 64, 30 };
    representation::expansion<16> e_7_result{ "2", "", "2" };
    representation::expansion<16> expansion_7 = representation::expand<16>(fraction_7);
    std::print("\nDane wejsciowe: {},\nCo chcemy otrzymac: {},\nCo otrzymujemy {}\n", fraction_7, e_7_result, expansion_7);

    std::print("\n\n-----------------\n\n");

    representation::expansion<10> expansion_2{ "1", "0", "1" };
    representation::fraction<int> f_2_result{ 91, 90 };
    representation::fraction<int> fraction_2 = representation::dexpand<10, int>(expansion_2);
    std::print("\nDane wejsciowe: {},\nCo chcemy otrzymac: {},\nCo otrzymujemy {}\n", expansion_2, f_2_result, fraction_2);

    representation::expansion<2> expansion_4{ "1", "00", "01" };
    representation::fraction<int> f_4_result{ 13, 12 };
    representation::fraction<int> fraction_4 = representation::dexpand<2, int>(expansion_4);
    std::print("\nDane wejsciowe: {},\nCo chcemy otrzymac: {},\nCo otrzymujemy {}\n", expansion_4, f_4_result, fraction_4);

    representation::expansion<8> expansion_6{ "10", "01", "12" };
    representation::fraction<int> f_6_result{ 32329, 4032 };
    representation::fraction<int> fraction_6 = representation::dexpand<8, int>(expansion_6);
    std::print("\nDane wejsciowe: {},\nCo chcemy otrzymac: {},\nCo otrzymujemy {}\n", expansion_6, f_6_result, fraction_6);

    representation::expansion<16> expansion_8{ "10", "ca", "0" };
    representation::fraction<int> f_8_result{ 2149, 128 };
    representation::fraction<int> fraction_8 = representation::dexpand<16, int>(expansion_8);
    std::print("\nDane wejsciowe: {},\nCo chcemy otrzymac: {},\nCo otrzymujemy {}\n", expansion_8, f_8_result, fraction_8);
}