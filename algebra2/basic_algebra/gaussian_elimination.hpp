#pragma once

#include "matrix.hpp"
#include <cassert>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <numeric>
#include <typeinfo>
#include <vector>
#include <utility>

namespace algorithms::gaussian_elimination
{

    enum class operation : std::uint8_t
    {
        none = 0,
        swap = 1 << 0,
        multiply = 1 << 1,
        add = 1 << 2,
    };

    template <std::integral I>
    struct fraction
    {
        I numerator{};
        I denominator{1};

        constexpr fraction<I> &operator-=(I i)
        {
            numerator -= i * denominator;
            return *this;
        }
        /*
        description:
            reduce fraction
    */
        void reduce()
        {
            I divider = std::gcd(numerator, denominator);
            if (divider != 0)
            {
                numerator /= divider;
                denominator /= divider;
            }
        }
    };

    template <std::integral I>
    inline bool operator==(fraction<I> f, fraction<I> g)
    {
        return f.numerator == g.numerator && f.denominator == g.denominator;
    }

    /*
        description:
            operations + - * / += -= *= /= on fractions
    */
    template <std::integral I>
    constexpr fraction<I> operator+(fraction<I> f, fraction<I> g)
    {
        fraction<I> result{};
        result.numerator =
            f.numerator * g.denominator + g.numerator * f.denominator;
        result.denominator = f.denominator * g.denominator;
        result.reduce();
        return result;
    }

    template <std::integral I>
    constexpr fraction<I> operator+=(fraction<I> &f, fraction<I> g)
    {
        f.numerator =
            f.numerator * g.denominator + g.numerator * f.denominator;
        f.denominator = f.denominator * g.denominator;
        f.reduce();
        return f;
    }

    template <std::integral I>
    constexpr fraction<I> operator-(fraction<I> f, fraction<I> g)
    {
        fraction<I> result;
        result.numerator = f.numerator * g.denominator - g.numerator * f.denominator;
        result.denominator = f.denominator * g.denominator;
        result.reduce();
        return result;
    }

    template <std::integral I>
    constexpr fraction<I> operator-=(fraction<I> &f, fraction<I> g)
    {
        f.numerator = f.numerator * g.denominator - g.numerator * f.denominator;
        f.denominator = f.denominator * g.denominator;
        f.reduce();
        return f;
    }

    template <std::integral I>
    constexpr fraction<I> operator*(fraction<I> f, fraction<I> g)
    {
        fraction<I> result;
        result.numerator = f.numerator * g.numerator;
        result.denominator = f.denominator * g.denominator;
        result.reduce();
        return result;
    }

    template <std::integral I>
    constexpr fraction<I> operator*=(fraction<I> &f, fraction<I> g)
    {
        f.numerator = f.numerator * g.numerator;
        f.denominator = f.denominator * g.denominator;
        f.reduce();
        return f;
    }

    template <std::integral I>
    constexpr fraction<I> operator/(fraction<I> f, fraction<I> g)
    {
        fraction<I> result;
        result.numerator = f.numerator * g.denominator;
        result.denominator = f.denominator * g.numerator;
        result.reduce();
        return result;
    }

    template <std::integral I>
    constexpr fraction<I> operator/=(fraction<I> &f, fraction<I> g)
    {
        fraction<I> result;
        f.numerator = f.numerator * g.denominator;
        f.denominator = f.denominator * g.numerator;
        f.reduce();
        return f;
    }

    template <std::integral I>
    constexpr fraction<I> operator*(fraction<I> f, int g)
    {
        f.numerator = f.numerator * g;
        f.reduce();
        return f;
    }

    template <std::integral I>
    constexpr fraction<I> operator*=(fraction<I> &f, int g)
    {
        f.numerator = f.numerator * g;
        f.reduce();
        return f;
    }

}
/*
    description:
        enables formatting of output data
        for algorirthms::gaussian_elimination::fraction<I>
*/
template <std::integral I>
struct std::formatter<algorithms::gaussian_elimination::fraction<I>>
{

    template <typename FormatParseContext>
    constexpr auto parse(FormatParseContext &ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const algorithms::gaussian_elimination::fraction<I> &f,
                FormatContext &ctx) const
    {
        return std::format_to(ctx.out(), "{}/{}", f.numerator, f.denominator);
    }
};

namespace algorithms::gaussian_elimination
{

    /*
        description:
            subtracts row_j multiplied by alpha from row_i
    */
    template <typename T, typename LP, typename R>
    auto subtract(ranges::matrix_view<T, LP> m, int row_i, int row_j, R alpha)
    {
        for (int i = 0; i < m.extent(1); i++)
        {
            m[row_j, i] *= alpha;
            m[row_i, i] -= m[row_j, i];
            m[row_j, i] /= alpha;
        }
    }

    /*
        description:
            swaps rows of a matrix
    */
    template <typename T, typename LP>
    auto swap(ranges::matrix_view<T, LP> m, int row_i, int row_j)
    {
        T temp;
        for (int i = 0; i < m.extent(1); i++)
        {
            temp = m[row_i, i];
            m[row_i, i] = m[row_j, i];
            m[row_j, i] = temp;
        }
    }
    template <typename T>
    struct is_fraction : std::false_type
    {
    };

    template <std::integral I>
    struct is_fraction<fraction<I>> : std::true_type
    {
    };

    template <typename T>
    constexpr bool is_fraction_v = is_fraction<std::remove_cvref_t<T>>::value;

    template <typename T>
    concept Fraction = is_fraction_v<T>;

    /*
        description:
            converts vector of ints to vector of fractions
    */
    template <std::ranges::range R>
        requires(std::integral<std::ranges::range_value_t<R>>)
    auto convert_to_vector_of_fractions(R &v)
    {
        if constexpr (Fraction<std::ranges::range_value_t<R>>)
        {
            return v;
        }
        else
        {
            return v | std::views::transform([](auto i)
                                             { return fraction{i, 1}; }) |
                   std::ranges::to<std::vector>();
        }
    }

    /*
        description:
            converts matrix of ints to vector of fractions
    */
    template <typename I, typename LP>
    auto convert_to_matrix_of_fractions(::ranges::matrix_view<I, LP> &m)
    {
        if constexpr (Fraction<I>)
        {
            std::vector<I> copy(m.begin(), m.end());
            ::ranges::matrix_view<I, LP> mat{copy, m.extent(0), m.extent(1), LP{}};
            return std::pair{std::move(copy), mat};
        }
        else
        {
            auto fracs{convert_to_vector_of_fractions(m)};
            ranges::matrix_view<fraction<I>, LP> matrix_of_fracs{fracs, m.extent(0), m.extent(1), LP{}};
            return std::pair{std::move(fracs), matrix_of_fracs};
        }
    }

    /*
        description:
            overload to avoid errors while converting matrix of fractions to a matrix of fractions
    */
    template <typename I, typename LP>
    auto convert_to_matrix_of_fractions(::ranges::matrix_view<fraction<I>, LP> &m)
    {
        return std::pair{std::move(m | std::ranges::to<std::vector>()), m};
    }

    /*
        description:
            generates an identity matrix depending on values of rows and cols
    */
    template <typename T, typename LP>
    auto eye(std::size_t rows, std::size_t cols)
    {
        int dimensions = rows * cols;
        std::vector<T> identity_vector;
        identity_vector.reserve(dimensions);

        for (int i = 0; i < dimensions; i++)
        {
            identity_vector[i] = T{0};
        }
        for (int i = 0; i < dimensions; i += rows + 1)
        {
            identity_vector[i] = T{1};
        }

        ranges::matrix_view<T, LP> identity_matrix(identity_vector, rows, cols, LP{});
        return std::pair{std::move(identity_vector), identity_matrix};
    }

    /*
       description:
           combines two matrices into one augmented matrix
   */
    template <typename T, typename LP, typename LP2>
    auto combine_matrices(ranges::matrix_view<T, LP> m1, ranges::matrix_view<T, LP2> m2)
    {
        int combined_dims = m1.extent(1) + m2.extent(1);
        int vector_size = combined_dims * m1.extent(0);

        std::vector<T> combined_vector;
        combined_vector.reserve(vector_size);

        for (int i = 0; i < m1.extent(0); i++)
        {
            for (int j = 0; j < m1.extent(1); j++)
            {
                combined_vector.push_back(m1[i, j]);
            }
            for (int k = 0; k < m2.extent(1); k++)
            {
                combined_vector.push_back(m2[i, k]);
            }
        }
        ranges::matrix_view combined_m(combined_vector, m1.extent(0), combined_dims, layout::row);
        return std::pair{std::move(combined_vector), combined_m};
    }

    /*
        description:
            splits one matrix in half and returns the rightmost half
    */
    template <typename T, typename LP>
    auto split_matrix(ranges::matrix_view<T, LP> m)
    {
        int rows = m.extent(0),
            cols = m.extent(1),
            dimensions = rows * cols;
        std::vector<T> split_vec;
        split_vec.reserve(dimensions / 2);

        for (int i = 0; i < rows; i++)
        {
            for (int j = cols / 2; j < cols; j++)
            {
                split_vec.push_back(m[i, j]);
            }
        }
        ranges::matrix_view split_mat(split_vec, rows, cols / 2, layout::row);
        return std::pair{std::move(split_vec), split_mat};
    }

    constexpr auto operator|(operation o1, operation o2) -> operation
    {
        return static_cast<operation>(std::to_underlying(o1) |
                                      std::to_underlying(o2));
    }

    constexpr auto operator&(operation o1, operation o2) -> operation
    {
        return static_cast<operation>(std::to_underlying(o1) &
                                      std::to_underlying(o2));
    }

    enum class axis
    {
        rows,
        columns,
    };

    enum class reducted_form
    {
        echelon,
        diagonal
    };

    enum class error
    {
        not_invertible,
        not_square,
    };

    /*
        description:
            struct which holds return values for gaussian elimination function
    */
    template <typename LP>
    struct gaussian_alg_result
    {
        std::vector<std::string> reduction_steps{};
        fraction<int> determinant_m{1, 1};

        std::vector<fraction<int>> reduced_matrix_vector{};
        std::size_t rows{1},
            cols{1};
        ranges::matrix_view<fraction<int>, LP> reduced_matrix{reduced_matrix_vector, rows, cols, layout::row};

        std::pair<std::vector<fraction<int>>, ranges::matrix_view<fraction<int>, LP>> matrix_range{std::move(reduced_matrix_vector), reduced_matrix};
    };

    /*
        description:
            helper function to reduce matrix m to echelon form
    */
    template <typename T, typename LP>
    auto gaussian_echelon(ranges::matrix_view<T, LP> m, gaussian_alg_result<LP> &result)
    {
        int rows = m.extent(0),
            cols = m.extent(1);
        fraction<int> factor;
        for (int i = 0; i < rows; i++)
        {
            if (m[i, i].numerator == 0)
            {
                for (int j = i + 1; j < rows; j++)
                {
                    if (m[j, i].numerator != 0)
                    {
                        swap(m, i, j);
                        result.reduction_steps.push_back(std::format("swap R{} with R{}", j + 1, i + 1));
                        result.determinant_m *= -1;
                    }
                }
            }
            if (m[i, i].numerator != 0)
            {
                for (int k = i + 1; k < rows; k++)
                {
                    factor = m[k, i] / m[i, i];
                    if (factor.numerator != 0)
                    {
                        subtract(m, k, i, factor);
                        result.reduction_steps.push_back(std::format("R{} - {} * R{}", k + 1, factor, i + 1));
                    }
                }
            }
        }
    }

    /*
        description:
            helper function to reduce matrix m to diagonal form
    */
    template <typename T, typename LP>
    auto gaussian_diagonal(ranges::matrix_view<T, LP> m, gaussian_alg_result<LP> &result)
    {
        int rows = m.extent(0),
            cols = m.extent(1);
        fraction<int> factor;
        for (int i = 0; i < rows; i++)
        {
            if (m[i, i].numerator != 0)
            {
                for (int k = i + 1; k < rows; k++)
                {
                    if (m[k, k].numerator != 0)
                    {
                        factor = m[i, k] / m[k, k];
                        if (factor.numerator != 0)
                        {
                            subtract(m, i, k, factor);
                        }
                    }
                }
            }
        }
        for (int i = 0; i < rows; i++)
        {
            factor = m[i, i];
            if (factor.numerator != 0)
            {
                for (int j = 0; j < cols; j++)
                {
                    m[i, j] /= factor;
                }
            }
            result.determinant_m *= factor;
        }
    }

    /*
        description:
            performs gaussian elimination on matrix m and returns it's determinant, reduced matrix and steps
    */
    template <typename T, typename LP>
    auto gaussian_elimiantion_alg(ranges::matrix_view<T, LP> m,
                                  axis a,
                                  reducted_form reducted,
                                  operation allowed_operations = operation::swap | operation::add | operation::multiply)
    {
        gaussian_alg_result<LP> result;
        auto [fracs, matrix_of_fracs] = convert_to_matrix_of_fractions(m);
        result.rows = matrix_of_fracs.extent(0);
        result.cols = matrix_of_fracs.extent(1);

        gaussian_echelon(matrix_of_fracs, result);

        if (reducted == reducted_form::diagonal)
        {
            gaussian_diagonal(matrix_of_fracs, result);
        }
        result.matrix_range = {std::move(fracs), matrix_of_fracs};
        return result;
    }

    /*
        description:
            runs gaussian elimination on axis a for matrix m until the reducted form reducted is obtained
    */
    template <typename T, typename LP>
    auto run(ranges::matrix_view<T, LP> m,
             axis a,
             reducted_form reducted = reducted_form::echelon,
             operation allowed_operations = operation::swap | operation::add | operation::multiply)
    {
        auto [m_reduced_vector, m_reduced] = gaussian_elimiantion_alg(m, axis::rows, reducted).matrix_range;

        return std::pair{std::move(m_reduced_vector), m_reduced};
    }

    /*
        description:
            runs gaussian elimination on axis a for matrix m until the reducted form reducted is obtained
            demonstrates on screen intermediate steps of the elimination
    */
    template <typename T, typename LP>
    auto show_steps(ranges::matrix_view<T, LP> m,
                    axis a,
                    reducted_form reducted = reducted_form::echelon,
                    operation allowed_operations = operation::swap | operation::add | operation::multiply) -> void
    {
        std::vector<std::string> steps = gaussian_elimiantion_alg(m, axis::rows, reducted).reduction_steps;
        std::print("{}\n", steps);
    }

    /*
        description:
            calculates the determinant of matrix m
    */
    template <typename T, typename LP>
    auto determinant(ranges::matrix_view<T, LP> m) -> std::expected<fraction<int>, error>
    {
        int rows = m.extent(0),
            cols = m.extent(1);

        if (rows != cols)
        {
            return std::unexpected(error::not_square);
        }
        else
        {
            return gaussian_elimiantion_alg(m, axis::rows, reducted_form::diagonal).determinant_m;
        }
    }

    /*
        description:
            returns inverse of matrix m
    */
    template <typename T, typename LP>
    auto inverse(ranges::matrix_view<T, LP> &m) -> std::expected<std::pair<std::vector<fraction<int>>, ranges::matrix_view<fraction<int>, std::layout_right>>, error>
    {
    std:
        size_t rows = m.extent(0),
               cols = m.extent(1);
        auto [identity_vector, identity_matrix] = eye<T, LP>(rows, cols);
        auto [combined_vec, combined_matrix] = combine_matrices(m, identity_matrix);
        if (rows != cols)
        {
            return std::unexpected(error::not_square);
        }
        else
        {
            if (determinant(m).value().numerator != 0)
            {
                auto [reduced_vector, reduced_matrix] = gaussian_elimiantion_alg(combined_matrix, axis::rows, reducted_form::diagonal).matrix_range;
                auto [inverse_vector, inverse_matrix] = split_matrix(reduced_matrix);
                return std::pair{std::move(inverse_vector), inverse_matrix};
            }
            else
            {
                return std::unexpected(error::not_invertible);
            }
        }
    }

} // namespace algorithms::gaussian_elimination

/*
    description:
        examples of using different gaussian elimination operations on matrices
*/
auto examples()
{
    std::vector v1{1, 1, -9, 6, 7, 10, 0, 0, -6, 4, 2, 3, 0, 0, -3, 2};
    std::vector v2{1, -3, -3, -1, 1, 2, 2, -3, -14};
    std::vector<algorithms::gaussian_elimination::fraction<int>> v3{{1, 2}, {1, 2}, {1, 2}, {1, 2}, {2, 4}, {1, 3}, {5, 3}, {1, 2}, {9, 4}};
    std::vector<algorithms::gaussian_elimination::fraction<int>> v4{{1, 3}, {1, 5}, {9, 2}, {5, 6}, {6, 7}, {11, 14}, {15, 10}, {0, 1}, {9, 1}};

    ranges::matrix_view m1(v1, 4, 4, layout::row);
    ranges::matrix_view m2(v2, 3, 3, layout::row);
    ranges::matrix_view m3(v3, 3, 3, layout::row);
    ranges::matrix_view m4(v4, 3, 3, layout::row);

    std::println("//////Gaussian Elimination Examples//////\n");
    std::println("m1 = {}\n", m1);
    auto [reduced, m1_reduced] = algorithms::gaussian_elimination::run(m1, algorithms::gaussian_elimination::axis::rows);
    std::println("m1 reduced = {}\n", m1_reduced);
    std::println("Steps of reducing m2 to row echelon form:");
    algorithms::gaussian_elimination::show_steps(m2, algorithms::gaussian_elimination::axis::rows);
    std::println("m3 = {}", m3);
    auto det = algorithms::gaussian_elimination::determinant(m3).value();
    std::println("Determinant of m3 = {}\n", det);
    std::println("m4 = {}\n", m4);
    auto [inverse_vector, inverse_m4] = algorithms::gaussian_elimination::inverse(m4).value();
    std::println("Inverse of matrix m4 = {}\n", inverse_m4);
}

/*
    description:
        provides testing of the functions using gaussian elimination
*/
auto tests()
{
    std::vector<algorithms::gaussian_elimination::fraction<int>> v1{{4, 1}, {3, 4}, {5, 9}, {5, 9}, {4, 7}, {5, 6}, {2, 6}, {3, 2}, {4, 3}};
    std::vector<algorithms::gaussian_elimination::fraction<int>> expected_v1{{246, 979}, {84, 979}, {-155, 979}, {700, 2937}, {-7784, 2937}, {13720, 8811}, {324, -979}, {-2898, -979}, {942, -979}};
    std::vector<algorithms::gaussian_elimination::fraction<int>> v1_for_column{{4, 1}, {3, 4}, {5, 9}, {5, 9}, {4, 7}, {5, 6}, {2, 6}, {3, 2}, {4, 3}};
    std::vector<algorithms::gaussian_elimination::fraction<int>> v1_column_expected{{246, 979}, {700, 2937}, {-324, 979}, {84, 979}, {-7784, 2937}, {2898, 979}, {155, -979}, {-13720, -8811}, {942, -979}};
    std::vector v2{1, 1, -9, 6, 7, 10, 0, 0, -6, 4, 2, 3, 0, 0, -3, 2};
    std::vector<algorithms::gaussian_elimination::fraction<int>> expected_v2{{-1, -1}, {-1, -1}, {9, -1}, {-6, -1}, {0, 1}, {3, 1}, {63, 1}, {-42, 1}, {0, 1}, {0, 1}, {-262, 1}, {179, 1}, {0, -1}, {0, -1}, {0, -1}, {13, -262}};
    std::vector v3{4, 10, 1, 3, 6, 7, 2, 4, 1, 6, 7, 2, 2, 5, 3, 6, 7, 9, 0, 7, 4, 2, 5, 4, 4};
    algorithms::gaussian_elimination::fraction<int> expected_det{5043, 1};

    ranges::matrix_view m1(v1, 3, 3, layout::row);
    ranges::matrix_view m1_column(v1_for_column, 3, 3, layout::column);
    ranges::matrix_view m2(v2, 4, 4, layout::row);
    ranges::matrix_view m3(v3, 5, 5, layout::column);
    std::println("//////Gaussian Elimination Tests//////\n");
    std::println("m1 = {}\n", m1);

    auto [m1_inverse_vector, m1_inverse] = algorithms::gaussian_elimination::inverse(m1).value();

    std::println("m1 inverse = {}\n", m1_inverse);
    std::vector m1_inv_vec = m1_inverse | std::ranges::to<std::vector>();
    assert((m1_inv_vec == expected_v1));

    std::println("m1 using column layout {}\n", m1_column);
    auto [m1_inverse_vector_column, m1_column_inverse] = algorithms::gaussian_elimination::inverse(m1_column).value();
    std::println("m1 inverse using column layout {}\n", m1_column_inverse);

    std::vector m1_clmn_inv = m1_column_inverse | std::ranges::to<std::vector>();
    assert((m1_clmn_inv == v1_column_expected));

    std::println("m2 = {}\n", m2);
    auto [m2_reduced_vector, m2_reduced] = algorithms::gaussian_elimination::run(m2, algorithms::gaussian_elimination::axis::rows);
    std::println("reduced m2 = {}\n", m2_reduced);

    std::vector m2_red_vec = m2_reduced | std::ranges::to<std::vector>();
    assert((m2_red_vec == expected_v2));

    std::println("m3 = {}\n", m3);
    auto det = algorithms::gaussian_elimination::determinant(m3).value();
    std::println("Determinant of m3 = {}\n", det);
    assert((det == expected_det));
}
