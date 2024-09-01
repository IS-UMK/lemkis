module;
#include <iostream>
#include <numeric>
#include <cmath>

export module fraction;

export template <std::integral I>
    class fraction {
        public:
        I numerator{1};
        I denominator{1};


        /*
        description:
            reduce fraction
        */
        void reduce() {
            I divider = std::gcd(numerator, denominator);
            if (divider != 0) {
                numerator /= divider;
                denominator /= divider;
            }
            if ((numerator < 0 && denominator < 0) ||
                (numerator > 0 && denominator < 0)) {
                numerator = -numerator;
                denominator = -denominator;
            }
        }
    };

    export template <std::integral I>
    inline auto operator==(fraction<I> f, fraction<I> g) -> bool {
        return f.numerator == g.numerator && f.denominator == g.denominator;
    }
    
    
    export template <std::integral I>
    inline auto operator!=(fraction<I> f, fraction<I> g) -> bool {
        return f.numerator != g.numerator && f.denominator != g.denominator;
    }


    template <std::integral I>
    struct std::formatter<fraction<I>> {

    template <typename FormatParseContext>
    constexpr auto parse(FormatParseContext& ctx) {
        return ctx.begin();
    }


    template <typename FormatContext>
    auto format(const fraction<I>& f,
                FormatContext& ctx) const {
        return std::format_to(ctx.out(), "{}/{}", f.numerator, f.denominator);
    }
    };

    /*
        description:
            operations + - * / += -= *= /= on fractions
    */
    export template <std::integral I>
    constexpr auto operator+(fraction<I> f, fraction<I> g) -> fraction<I> {
        fraction<I> result{};
        result.numerator =
            f.numerator * g.denominator + g.numerator * f.denominator;
        result.denominator = f.denominator * g.denominator;
        result.reduce();
        return result;
    }

    export template <std::integral I>
    constexpr auto operator+=(fraction<I>& f, fraction<I> g) -> fraction<I> {
        f.numerator = f.numerator * g.denominator + g.numerator * f.denominator;
        f.denominator = f.denominator * g.denominator;
        f.reduce();
        return f;
    }

    export template <std::integral I>
    constexpr auto operator-(fraction<I> f, fraction<I> g) -> fraction<I> {
        fraction<I> result;
        result.numerator =
            f.numerator * g.denominator - g.numerator * f.denominator;
        result.denominator = f.denominator * g.denominator;
        result.reduce();
        return result;
    }

    export template <std::integral I>
    constexpr auto operator-=(fraction<I>& f, fraction<I> g) -> fraction<I> {
        f.numerator = f.numerator * g.denominator - g.numerator * f.denominator;
        f.denominator = f.denominator * g.denominator;
        f.reduce();
        return f;
    }

    export template <std::integral I>
    constexpr auto operator*(fraction<I> f, fraction<I> g) -> fraction<I> {
        fraction<I> result;
        result.numerator = f.numerator * g.numerator;
        result.denominator = f.denominator * g.denominator;
        result.reduce();
        return result;
    }

    export template <std::integral I>
    constexpr auto operator*=(fraction<I>& f, fraction<I> g) -> fraction<I> {
        f.numerator = f.numerator * g.numerator;
        f.denominator = f.denominator * g.denominator;
        f.reduce();
        return f;
    }

    export template <std::integral I>
    constexpr auto operator/(fraction<I> f, fraction<I> g) -> fraction<I> {
        fraction<I> result;
        result.numerator = f.numerator * g.denominator;
        result.denominator = f.denominator * g.numerator;
        result.reduce();
        return result;
    }

    export template <std::integral I>
    constexpr auto operator/=(fraction<I>& f, fraction<I> g) -> fraction<I> {
        f.numerator = f.numerator * g.denominator;
        f.denominator = f.denominator * g.numerator;
        f.reduce();
        return f;
    }

    export template <std::integral I>
    constexpr auto operator*(fraction<I> f, I g) -> fraction<I> {
        f.numerator = f.numerator * g;
        f.reduce();
        return f;
    }

    export template <std::integral I>
    constexpr auto operator*=(fraction<I>& f, I g) -> fraction<I> {
        f.numerator = f.numerator * g;
        f.reduce();
        return f;
    }

    export template <std::integral I>
    constexpr auto operator/=(fraction<I>& f, I g) -> fraction<I> {
        f.numerator = f.numerator / g;
        f.reduce();
        return f;
    }

    export template <typename T>
    struct is_fraction : std::false_type {};


    export template <std::integral I>
    struct is_fraction<fraction<I>> : std::true_type {};


    export template <typename T>
    constexpr bool is_fraction_v = is_fraction<std::remove_cvref_t<T>>::value;


    export template <typename T>
    concept check_fraction = is_fraction_v<T>;