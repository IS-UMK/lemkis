module;
#include <cassert>
#include <cmath>
#include <cstdint>
#include <expected>
#include <iostream>
#include <numeric>
#include <typeinfo>
#include <utility>
#include <vector>
#include <print>

export module gaussian_elimination;

import matrix;
import fraction;

export namespace utils::matrix{


    /*
       description:
           combines two matrices into one augmented matrix 
           where matrix m1 is on the right and matrix m2 is on the left of the combined matrix
   */
    template <typename T>
    auto combine_matrices(::matrix<T> m1, ::matrix<T> m2){
        const std::size_t combined_columns = m1.number_of_columns() + m2.number_of_columns();
        ::matrix<T> combined_matrix{m1.number_of_rows(), combined_columns, {0}};
        for(std::size_t i = 0; i < m1.number_of_rows(); i++){
            for(std::size_t j = 0; j < m1.number_of_columns(); j++){
                combined_matrix[i, j] = m1[i , j];
            }
        }
        for(std::size_t i = 0; i < m2.number_of_rows(); i++){
            for(std::size_t j = m1.number_of_columns(); j < combined_columns; j++){
                combined_matrix[i, j] = m2[i , j - m1.number_of_columns()];
            }
        }
        return combined_matrix;
    }


    /*
        description:
            splits a matrix in half and returns the rightmost half
    */
    template <typename T>
    auto split_matrix(::matrix<T> m){
        const std::size_t split_columns = m.number_of_columns() / 2;
        ::matrix<T> split_mat{m.number_of_rows(), split_columns, {0}};
        for(std::size_t i = 0; i < m.number_of_rows(); i++){
            for(std::size_t j = 0; j < split_columns; j++){
                split_mat[i, j] = m[i, j + split_columns];
            }
        }
        return split_mat;
    }

    template <typename R>
    auto to_fractions_helper(R& m){
        ::matrix<fraction<int>> converted_matrix{m.number_of_rows(), m.number_of_columns(), {0}}; 
        for(std::size_t i = 0; i < m.number_of_rows(); i++){
                for(std::size_t j = 0; j < m.number_of_columns(); j++){
                    converted_matrix[i, j] = fraction{m[i, j], 1};
                }
            }
            return converted_matrix;
    }

    /*
        description:
            converts matrix of ints to matrix of fractions
    */
    template <typename R>
    auto to_matrix_of_fractions(R& m) {
        if constexpr (check_fraction<std::ranges::range_value_t<R>>) {
            return m;
        } else {
            to_fractions_helper(m);
        }
    }


    enum class reducted_form : std::uint8_t { echelon, echelon_reduced };
    enum class error : std::uint8_t {
        not_invertible,
        not_square,
    };


    /*
        description:
            struct which holds return values for gaussian elimination function
    */
    struct gaussian_alg_result {
        std::vector<std::string> reduction_steps;
        fraction<int> determinant_m{1, 1};

        std::size_t rows{1}, cols{1};
        ::matrix<fraction<int>> reduced_matrix{
            rows,
            cols, {0}};

        // std::pair<std::vector<fraction<int>>,
        //           matrix<fraction<int>, LP>>
        //     matrix_range{std::move(reduced_matrix_vector), reduced_matrix};
    };


    /*
        description:
            helper function for gaussian_echelon function to swap rows
    */
    template <typename T>
    auto gaussian_echelon_swap(::matrix<T>& m,
                               gaussian_alg_result& result,
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
    template <typename T>
    auto gaussian_echelon_subtract(::matrix<T>& m,
                                   gaussian_alg_result& result,
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
    template <typename T>
    auto gaussian_echelon(::matrix<T>& m,
                          gaussian_alg_result& result) {
        const std::size_t rows = m.number_of_rows();
        for (std::size_t i = 0; i < rows; i++) {
            if (m[i, i].numerator == 0) {
                gaussian_echelon_swap(m, result, i);
            }
            if (m[i, i].numerator != 0) {
                gaussian_echelon_subtract(m, result, i);
            }
        }
    }


    /*
        description:
            helper function for gaussian_diagonal_subtract to comply with clang-tidy requirements
    */
    template <typename T>
    auto diagonal_subtract_helper(::matrix<T>& m,
                                    gaussian_alg_result& result,
                                    std::size_t rows,
                                    std::size_t iterator){
        fraction<int> factor;
        for (std::size_t k = iterator + 1; k < rows; k++) {
            if (m[iterator, k].numerator != 0 && m[k, k].numerator != 0) {
                factor = m[iterator, k] / m[k, k];
                subtract(m, iterator, k, factor);
                result.reduction_steps.push_back(std::format(
                    "R{} - {} * R{}", k + 1, factor, iterator + 1));
            }
        }
    }


    /*
        description:
            helper function for gaussian_diagonal to subtract rows
    */
    template <typename T>
    auto gaussian_diagonal_subtract(::matrix<T>& m,
                                    gaussian_alg_result& result) {
        const std::size_t rows = m.number_of_rows();
        for (std::size_t i = 0; i < rows; i++) {
            if (m[i, i].numerator != 0) {
                diagonal_subtract_helper(m, result, rows, i);
            }
        }
    }


    /*
        description:
            helper function for gaussian_diagonal_divide_by_factor to comply with clang-tidy requirements
    */
    template <typename T>
    auto divide_by_factor_helper(::matrix<T>& m,
                                fraction<int> factor,
                                std::size_t iterator){
        const std::size_t cols = m.number_of_columns();
        if (factor.numerator != 0) {
            for (std::size_t j = 0; j < cols; j++) { m[iterator, j] /= factor; }
        }
    }


    /*
        description:
            helper function for gaussian_diagonal to divide each diagonal element by itself
    */
    template <typename T>
    auto gaussian_diagonal_divide_by_factor(::matrix<T>& m,
                                            gaussian_alg_result& result) {
        const std::size_t rows = m.number_of_rows();
        fraction<int> factor;
        for (std::size_t i = 0; i < rows; i++) {
            factor = m[i, i];
            divide_by_factor_helper(m, factor, i);
            result.determinant_m *= factor;
        }
    }


    /*
        description:
            helper function to reduce matrix m to diagonal form
    */
    template <typename T>
    auto gaussian_diagonal(::matrix<T>& m,
                           gaussian_alg_result& result) {
        gaussian_diagonal_subtract(m, result);
        gaussian_diagonal_divide_by_factor(m, result);
    }


    /*
        description:
            performs gaussian elimination on matrix m and returns it's
       determinant, reduced matrix and steps
    */
    template <typename T>
    auto gaussian_elimiantion_alg(
        ::matrix<T>& m,
        reducted_form reducted)
        -> gaussian_alg_result{
        gaussian_alg_result result;
        auto matrix_of_fracs = to_matrix_of_fractions(m);
        result.rows = matrix_of_fracs.number_of_rows();
        result.cols = matrix_of_fracs.number_of_columns();

        gaussian_echelon(matrix_of_fracs, result);

        if (reducted == reducted_form::echelon_reduced) {
            gaussian_diagonal(matrix_of_fracs, result);
        }
        result.reduced_matrix = matrix_of_fracs;
        
        return result;
    }


    /*
        description:
            runs gaussian elimination on axis a for matrix m until the reducted
       form reducted is obtained
    */
    template <typename T>
    auto run(::matrix<T> m,
             reducted_form reducted = reducted_form::echelon) {
        auto m_reduced = gaussian_elimiantion_alg(m, reducted).reduced_matrix;
        return m_reduced;
    }


    /*
        description:
            runs gaussian elimination on axis a for matrix m until the reducted
       form reducted is obtained demonstrates on screen intermediate steps of
       the elimination
    */
    template <typename T>
    auto show_steps(::matrix<T> m,
                    reducted_form reducted = reducted_form::echelon)
        -> std::vector<std::string> {
        std::vector<std::string> steps = gaussian_elimiantion_alg(m, reducted).reduction_steps;
        //std::print("{}\n", steps);
        return steps;
    }


    /*
        description:
            calculates the determinant of matrix m
    */
    template <typename T>
    auto determinant(::matrix<T> m)
        -> std::expected<fraction<int>, error> {
        const std::size_t rows = m.number_of_rows();
        const std::size_t cols = m.number_of_columns();

        if (rows != cols) { return std::unexpected(error::not_square); }
        return gaussian_elimiantion_alg(m, reducted_form::echelon_reduced).determinant_m;
    }


    /*
        description:
            returns inverse of matrix m
    */
    template <typename T>
    auto inverse(::matrix<T>& m) -> std::expected<::matrix<fraction<int>>, error> {
        std::size_t const rows = m.number_of_rows();
        std::size_t const cols = m.number_of_columns();
        if (rows != cols) { return std::unexpected(error::not_square); }
        auto identity_matrix = identity<T>(rows);
        auto combined_matrix =
            combine_matrices(m, identity_matrix);
        if (determinant(m).value().numerator != 0) {
            auto reduced_matrix =
                gaussian_elimiantion_alg(combined_matrix,
                                         reducted_form::echelon_reduced).reduced_matrix;
            auto inverse_matrix = split_matrix(reduced_matrix);
            return  inverse_matrix;
        }
        return std::unexpected(error::not_invertible);
    }
}
