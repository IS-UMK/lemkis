#pragma once

#include <cassert>
#include <cmath>
#include <cstdint>
#include <expected>
#include <iostream>
#include <numeric>
#include <typeinfo>
#include <utility>
#include <vector>

#include "matrix.hpp"


namespace algorithms::gaussian_elimination {

    enum class operation : std::uint8_t {
        none = 0,
        swap = 1 << 0,
        multiply = 1 << 1,
        add = 1 << 2,
    };


    template <std::integral I>
    struct fraction {
        I numerator{};
        I denominator{1};


        constexpr auto operator-=(I i) -> fraction<I>& {
            numerator -= i * denominator;
            return *this;
        }


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


    template <std::integral I>
    inline auto operator==(fraction<I> f, fraction<I> g) -> bool {
        return f.numerator == g.numerator && f.denominator == g.denominator;
    }


    /*
        description:
            operations + - * / += -= *= /= on fractions
    */
    template <std::integral I>
    constexpr auto operator+(fraction<I> f, fraction<I> g) -> fraction<I> {
        fraction<I> result{};
        result.numerator =
            f.numerator * g.denominator + g.numerator * f.denominator;
        result.denominator = f.denominator * g.denominator;
        result.reduce();
        return result;
    }


    template <std::integral I>
    constexpr auto operator+=(fraction<I>& f, fraction<I> g) -> fraction<I> {
        f.numerator = f.numerator * g.denominator + g.numerator * f.denominator;
        f.denominator = f.denominator * g.denominator;
        f.reduce();
        return f;
    }


    template <std::integral I>
    constexpr auto operator-(fraction<I> f, fraction<I> g) -> fraction<I> {
        fraction<I> result;
        result.numerator =
            f.numerator * g.denominator - g.numerator * f.denominator;
        result.denominator = f.denominator * g.denominator;
        result.reduce();
        return result;
    }


    template <std::integral I>
    constexpr auto operator-=(fraction<I>& f, fraction<I> g) -> fraction<I> {
        f.numerator = f.numerator * g.denominator - g.numerator * f.denominator;
        f.denominator = f.denominator * g.denominator;
        f.reduce();
        return f;
    }


    template <std::integral I>
    constexpr auto operator*(fraction<I> f, fraction<I> g) -> fraction<I> {
        fraction<I> result;
        result.numerator = f.numerator * g.numerator;
        result.denominator = f.denominator * g.denominator;
        result.reduce();
        return result;
    }


    template <std::integral I>
    constexpr auto operator*=(fraction<I>& f, fraction<I> g) -> fraction<I> {
        f.numerator = f.numerator * g.numerator;
        f.denominator = f.denominator * g.denominator;
        f.reduce();
        return f;
    }


    template <std::integral I>
    constexpr auto operator/(fraction<I> f, fraction<I> g) -> fraction<I> {
        fraction<I> result;
        result.numerator = f.numerator * g.denominator;
        result.denominator = f.denominator * g.numerator;
        result.reduce();
        return result;
    }


    template <std::integral I>
    constexpr auto operator/=(fraction<I>& f, fraction<I> g) -> fraction<I> {
        f.numerator = f.numerator * g.denominator;
        f.denominator = f.denominator * g.numerator;
        f.reduce();
        return f;
    }


    template <std::integral I>
    constexpr auto operator*(fraction<I> f, int g) -> fraction<I> {
        f.numerator = f.numerator * g;
        f.reduce();
        return f;
    }


    template <std::integral I>
    constexpr auto operator*=(fraction<I>& f, int g) -> fraction<I> {
        f.numerator = f.numerator * g;
        f.reduce();
        return f;
    }

}  // namespace algorithms::gaussian_elimination


/*
    description:
        enables formatting of output data
        for algorirthms::gaussian_elimination::fraction<I>
*/
template <std::integral I>
struct std::formatter<algorithms::gaussian_elimination::fraction<I>> {

    template <typename FormatParseContext>
    constexpr auto parse(FormatParseContext& ctx) {
        return ctx.begin();
    }


    template <typename FormatContext>
    auto format(const algorithms::gaussian_elimination::fraction<I>& f,
                FormatContext& ctx) const {
        return std::format_to(ctx.out(), "{}/{}", f.numerator, f.denominator);
    }
};

namespace algorithms::gaussian_elimination {

    /*
        description:
            subtracts row_j multiplied by alpha from row_i
    */
    template <typename T, typename LP, typename R>
    auto subtract(ranges::matrix_view<T, LP> m,
                  std::size_t row_i,
                  std::size_t row_j,
                  R alpha) {
        for (std::size_t i = 0; i < m.number_of_columns(); i++) {
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
    auto swap(ranges::matrix_view<T, LP> m,
              std::size_t row_i,
              std::size_t row_j) {
        T temp;
        for (std::size_t i = 0; i < m.number_of_columns(); i++) {
            temp = m[row_i, i];
            m[row_i, i] = m[row_j, i];
            m[row_j, i] = temp;
        }
    }


    template <typename T>
    struct is_fraction : std::false_type {};


    template <std::integral I>
    struct is_fraction<fraction<I>> : std::true_type {};


    template <typename T>
    constexpr bool is_fraction_v = is_fraction<std::remove_cvref_t<T>>::value;


    template <typename T>
    concept check_fraction = is_fraction_v<T>;


    /*
        description:
            converts vector of ints to vector of fractions
    */
    template <std::ranges::range R>
    requires(std::integral<std::ranges::range_value_t<
                 R>>) auto convert_to_vector_of_fractions(R& v) {
        if constexpr (check_fraction<std::ranges::range_value_t<R>>) {
            return v;
        } else {
            return v | std::views::transform([](auto i) {
                       return fraction{i, 1};
                   }) |
                   std::ranges::to<std::vector>();
        }
    }


    /*
        description:
            converts matrix of ints to vector of fractions
    */
    template <typename I, typename LP>
    auto convert_to_matrix_of_fractions(::ranges::matrix_view<I, LP>& m) {
        if constexpr (check_fraction<I>) {
            std::vector<I> copy(m.begin(), m.end());
            ::ranges::matrix_view<I, LP> mat{
                copy, m.extent(0), m.extent(1), LP{}};
            return std::pair{std::move(copy), mat};
        } else {
            auto fracs{convert_to_vector_of_fractions(m)};
            ranges::matrix_view<fraction<I>, LP> matrix_of_fracs{
                fracs, m.extent(0), m.extent(1), LP{}};
            return std::pair{std::move(fracs), matrix_of_fracs};
        }
    }


    /*
        description:
            generates an identity matrix of type T depending on values of rows and cols
    */
    template <typename T, typename LP>
    auto eye(std::size_t rows, std::size_t cols) {
        const std::size_t dimensions = rows * cols;
        std::vector<T> identity_vector;
        identity_vector.reserve(dimensions);

        for (std::size_t i = 0; i < dimensions; i++) {
            identity_vector[i] = T{0};
        }
        for (std::size_t i = 0; i < dimensions; i += rows + 1) {
            identity_vector[i] = T{1};
        }

        const ranges::matrix_view<T, LP> identity_matrix(
            identity_vector, rows, cols, LP{});
        return std::pair{std::move(identity_vector), identity_matrix};
    }


    /*
       description:
           combines two matrices into one augmented matrix 
           where matrix m1 is on the right and matrix m2 is on the left of the combined matrix
   */
    template <typename T, typename LP>
    auto combine_matrices(ranges::matrix_view<T, LP> m1,
                          ranges::matrix_view<T, LP> m2) {
        const std::size_t combined_dims = m1.number_of_columns() + m2.number_of_columns();
        const std::size_t vector_size = combined_dims * m1.number_of_rows();
        std::vector<T> combined_vector;
        combined_vector.reserve(vector_size);
        for (std::size_t i = 0; i < m1.number_of_rows(); i++) {
            for (std::size_t j = 0; j < m1.number_of_columns(); j++) {
                combined_vector.push_back(m1[i, j]);
            }
            for (std::size_t k = 0; k < m2.number_of_columns(); k++) {
                combined_vector.push_back(m2[i, k]);
            }
        }
        const ranges::matrix_view combined_m(combined_vector, m1.number_of_rows(), combined_dims, layout::row);
        return std::pair{std::move(combined_vector), combined_m};
    }


    /*
        description:
            splits one matrix in half and returns the rightmost half
    */
    template <typename T, typename LP>
    auto split_matrix(ranges::matrix_view<T, LP> m) {
        const std::size_t rows = m.number_of_rows();
        const std::size_t cols = m.number_of_columns();
        const std::size_t dimensions = rows * cols;
        std::vector<T> split_vec;
        split_vec.reserve(dimensions / 2);

        for (std::size_t i = 0; i < rows; i++) {
            for (std::size_t j = cols / 2; j < cols; j++) {
                split_vec.push_back(m[i, j]);
            }
        }
        const ranges::matrix_view split_mat(
            split_vec, rows, cols / 2, layout::row);
        return std::pair{std::move(split_vec), split_mat};
    }


    constexpr auto operator|(operation o1, operation o2) -> operation {
        return static_cast<operation>(std::to_underlying(o1) |
                                      std::to_underlying(o2));
    }


    constexpr auto operator&(operation o1, operation o2) -> operation {
        return static_cast<operation>(std::to_underlying(o1) &
                                      std::to_underlying(o2));
    }


    enum class reducted_form : std::uint8_t { echelon, diagonal };


    enum class error : std::uint8_t {
        not_invertible,
        not_square,
    };


    /*
        description:
            struct which holds return values for gaussian elimination function
    */
    template <typename LP>
    struct gaussian_alg_result {
        std::vector<std::string> reduction_steps;
        fraction<int> determinant_m{1, 1};

        std::vector<fraction<int>> reduced_matrix_vector;
        std::size_t rows{1}, cols{1};
        ranges::matrix_view<fraction<int>, LP> reduced_matrix{
            reduced_matrix_vector,
            rows,
            cols,
            layout::row};

        std::pair<std::vector<fraction<int>>,
                  ranges::matrix_view<fraction<int>, LP>>
            matrix_range{std::move(reduced_matrix_vector), reduced_matrix};
    };


    /*
        description:
            helper function for gaussian_echelon function to swap rows
    */
    template <typename T, typename LP>
    auto gaussian_echelon_swap(ranges::matrix_view<T, LP> m,
                               gaussian_alg_result<LP>& result,
                               std::size_t i) {
        const std::size_t rows = m.number_of_rows();
        for (std::size_t j = i + 1; j < rows; j++) {
            if (m[j, i].numerator != 0) {
                swap(m, i, j);
                result.reduction_steps.push_back(
                    std::format("swap R{} with R{}", j + 1, i + 1));
                result.determinant_m *= -1;
            }
        }
    }


    /*
        description:
            helper function for gaussian_echelon function to subtract rows
    */
    template <typename T, typename LP>
    auto gaussian_echelon_subtract(ranges::matrix_view<T, LP> m,
                                   gaussian_alg_result<LP>& result,
                                   std::size_t i) {
        const std::size_t rows = m.number_of_rows();
        fraction<int> factor;
        for (std::size_t k = i + 1; k < rows; k++) {
            factor = m[k, i] / m[i, i];
            if (factor.numerator != 0) {
                subtract(m, k, i, factor);
                result.reduction_steps.push_back(
                    std::format("R{} - {} * R{}", k + 1, factor, i + 1));
            }
        }
    }


    /*
        description:
            helper function to reduce matrix m to echelon form
    */
    template <typename T, typename LP>
    auto gaussian_echelon(ranges::matrix_view<T, LP> m,
                          gaussian_alg_result<LP>& result,
                          operation allowed_operations) {
        const std::size_t rows = m.number_of_rows();
        const bool is_add_allowed{(operation::add & allowed_operations) !=
                                  operation::none};
        const bool is_swap_allowed{(operation::swap & allowed_operations) !=
                                   operation::none};
        for (std::size_t i = 0; i < rows; i++) {
            if (m[i, i].numerator == 0 && is_swap_allowed) {
                gaussian_echelon_swap(m, result, i);
            }
            if (m[i, i].numerator != 0 && is_add_allowed) {
                gaussian_echelon_subtract(m, result, i);
            }
        }
    }


    // NOLINTBEGIN
    /*
        description:
            helper function for gaussian_diagonal to subtract rows
    */
    template <typename T, typename LP>
    auto gaussian_diagonal_subtract(ranges::matrix_view<T, LP> m,
                                    gaussian_alg_result<LP>& result) {
        const std::size_t rows = m.number_of_rows();
        fraction<int> factor;
        for (std::size_t i = 0; i < rows; i++) {
            if (m[i, i].numerator != 0) {
                for (std::size_t k = i + 1; k < rows; k++) {
                    if (m[k, k].numerator != 0) {
                        factor = m[i, k] / m[k, k];
                        if (factor.numerator != 0) {
                            subtract(m, i, k, factor);
                            result.reduction_steps.push_back(std::format(
                                "R{} - {} * R{}", k + 1, factor, i + 1));
                        }
                    }
                }
            }
        }
    }


    /*
        description:
            helper function for gaussian_diagonal to divide each diagonal element by itself
    */
    template <typename T, typename LP>
    auto gaussian_diagonal_divide_by_factor(ranges::matrix_view<T, LP> m,
                                            gaussian_alg_result<LP>& result) {
        const std::size_t rows = m.number_of_rows();
        const std::size_t cols = m.number_of_columns();
        fraction<int> factor;
        for (std::size_t i = 0; i < rows; i++) {
            factor = m[i, i];
            if (factor.numerator != 0) {
                for (std::size_t j = 0; j < cols; j++) { m[i, j] /= factor; }
            }
            result.determinant_m *= factor;
        }
    }
    // NOLINTEND


    /*
        description:
            helper function to reduce matrix m to diagonal form
    */
    template <typename T, typename LP>
    auto gaussian_diagonal(ranges::matrix_view<T, LP> m,
                           gaussian_alg_result<LP>& result,
                           operation allowed_operations) {
        const bool is_add_allowed{(operation::add & allowed_operations) !=
                                  operation::none};
        const bool is_swap_allowed{(operation::swap & allowed_operations) !=
                                   operation::none};
        if (is_add_allowed) { gaussian_diagonal_subtract(m, result); }
        if (is_swap_allowed) { gaussian_diagonal_divide_by_factor(m, result); }
    }


    /*
        description:
            performs gaussian elimination on matrix m and returns it's
       determinant, reduced matrix and steps
    */
    template <typename T, typename LP>
    auto gaussian_elimiantion_alg(
        ranges::matrix_view<T, LP> m,
        reducted_form reducted,
        operation allowed_operations = operation::swap | operation::add |
                                       operation::multiply)
        -> gaussian_alg_result<LP> {
        gaussian_alg_result<LP> result;
        auto [fracs, matrix_of_fracs] = convert_to_matrix_of_fractions(m);
        result.rows = matrix_of_fracs.number_of_rows();
        result.cols = matrix_of_fracs.number_of_columns();

        gaussian_echelon(matrix_of_fracs, result, allowed_operations);

        if (reducted == reducted_form::diagonal) {
            gaussian_diagonal(matrix_of_fracs, result, allowed_operations);
        }
        result.matrix_range = {std::move(fracs), matrix_of_fracs};
        return result;
    }


    /*
        description:
            runs gaussian elimination on axis a for matrix m until the reducted
       form reducted is obtained
    */
    template <typename T, typename LP>
    auto run(ranges::matrix_view<T, LP> m,
             reducted_form reducted = reducted_form::echelon,
             operation allowed_operations = operation::swap | operation::add |
                                            operation::multiply)
        -> std::pair<std::vector<fraction<T>>,
                     ranges::matrix_view<fraction<T>, LP>> {
        auto [m_reduced_vector, m_reduced] =
            gaussian_elimiantion_alg(m, reducted, allowed_operations)
                .matrix_range;

        return std::pair{std::move(m_reduced_vector), m_reduced};
    }


    /*
        description:
            runs gaussian elimination on axis a for matrix m until the reducted
       form reducted is obtained demonstrates on screen intermediate steps of
       the elimination
    */
    template <typename T, typename LP>
    auto show_steps(ranges::matrix_view<T, LP> m,
                    reducted_form reducted = reducted_form::echelon,
                    operation allowed_operations = operation::swap |
                                                   operation::add |
                                                   operation::multiply)
        -> void {
        std::vector<std::string> steps =
            gaussian_elimiantion_alg(m, reducted, allowed_operations)
                .reduction_steps;
        std::print("{}\n", steps);
    }


    /*
        description:
            calculates the determinant of matrix m
    */
    template <typename T, typename LP>
    auto determinant(ranges::matrix_view<T, LP> m)
        -> std::expected<fraction<int>, error> {
        const std::size_t rows = m.number_of_rows();
        const std::size_t cols = m.number_of_columns();

        if (rows != cols) { return std::unexpected(error::not_square); }
        return gaussian_elimiantion_alg(m, reducted_form::diagonal)
            .determinant_m;
    }


    /*
        description:
            returns inverse of matrix m
    */
    template <typename T, typename LP>
    auto inverse(ranges::matrix_view<T, LP>& m) -> std::expected<
        std::pair<std::vector<fraction<int>>,
                  ranges::matrix_view<fraction<int>, std::layout_right>>,
        error> {
        std::size_t const rows = m.number_of_rows();
        std::size_t const cols = m.number_of_columns();
        if (rows != cols) { return std::unexpected(error::not_square); }
        auto [identity_vector, identity_matrix] = eye<T, LP>(rows, cols);
        auto [combined_vec, combined_matrix] =
            combine_matrices(m, identity_matrix);
        if (determinant(m).value().numerator != 0) {
            auto [reduced_vector, reduced_matrix] =
                gaussian_elimiantion_alg(combined_matrix,
                                         reducted_form::diagonal).matrix_range;
            auto [inverse_vector, inverse_matrix] = split_matrix(reduced_matrix);
            return std::pair{std::move(inverse_vector), inverse_matrix};
        }
        return std::unexpected(error::not_invertible);
    }

}  // namespace algorithms::gaussian_elimination
