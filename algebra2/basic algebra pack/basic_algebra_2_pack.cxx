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

export module basic_algebra_pack;

import matrix;
import gaussian_elimination;


using matrix_t = matrix::matrix;
using matrix_view_t = matrix::matrix_view;
using layout_left = matrix::layout::row;
using layout_right = matrix::layout::column;
export namespace utils::matrix {

    /*
      description:
        Alias type for matrix with data vector and matrix view
    */

    /*
      description:
        Alias type for homogeneous solution
    */
    template <typename T>
    using homogeneous_solution_t = std::optional<matrix<T, std::layout_left>>;

    /*
      description:
        Conversion type to fraction
    */
    template <typename T>
    using to_fraction_type =
        std::conditional_t<algorithms::gaussian_elimination::is_fraction_v<T>,
                           t,
                           algorithms::gaussian_elimination::fraction<T>>;

    /*
      description:
        The function is used to find the first non-zero element in a specific
      row of the matrix
    */
    template <typename T, typename LP>
    auto first_non_zero(ranges::matrix_view<T, LP> m,
                        std::size_t row_index) -> std::size_t;

    /*
      description:
        The function checks whether there is a row in the given matrix that can
      be considered contradictory.
    */
    template <typename T, typename LP>
    auto is_contradictory(ranges::matrix_view<T, LP> combined) -> bool;

    /*
      description:
        The function it is used to find the indexes of the first non-zero
      elements (pivots) in each row of the matrix and return them as a vector.
    */
    template <typename T, typename LP>
    auto pivots(ranges::matrix_view<T, LP> combined)
        -> std::vector<std::size_t>;

    /*
      description:
        Function calculates a special solution of the system of linear equations
      represented by the matrix of coefficients and the vector of free words.
    */
    template <typename T, typename LP>
    inline auto special_solution(ranges::matrix_view<T, LP> coefficients,
                                 std::ranges::range auto y)
        -> std::optional<std::vector<to_fraction_type<T>>>;

    /*
      description:
        Structure that represents the result of the solution operation.
      members:
        bool exists - the logical variable indicates whether the result exists
        std::optional<std::vector<T>> special - If the solution operation
      generates special results, such as special values, they will be stored in
      this vector homogeneous_solution_t<T> homogeneous - This variable stores a
      homogeneous solution of the operation that was performed.
    */
    template <typename T>
    struct solve_result {
        bool exists{false};
        std::optional<std::vector<T>> special{};
        homogeneous_solution_t<T> homogeneous{};
    };

    /*
      description:
        A function that is designed to solve systems of linear equations in the
      form of Ax = y.
    */
    template <typename T, typename LP>
    inline auto solve(ranges::matrix_view<T, LP> coefficients,
                      std::ranges::range auto y)
        -> solve_result<to_fraction_type<T>>;

    /*
      description:
        Part of the function used to determine the matrix kernel coefficients
    */
    template <typename T, typename LP>
    inline auto kernel_3(ranges::matrix_view<T, LP> matrix,
                         int rows,
                         int columns,
                         int &l) -> std::vector<T>;

    /*
      description:
        Part of the function used to determine the matrix kernel coefficients
    */
    template <typename T, typename LP>
    inline auto kernel_2(ranges::matrix_view<T, LP> coefficients)
        -> matrix<to_fraction_type<T>, LP>;

    /*
      description:
        Function used to determine the matrix kernel coefficients
    */
    template <typename T, typename LP>
    inline auto kernel(ranges::matrix_view<T, LP> coefficients)
        -> std::pair<
            std::vector<to_fraction_type<T>>,
            ::ranges::matrix_view<to_fraction_type<T>, std::layout_left>>;

    /*
      description:
        The structure is used to represent the result of checking whether a
      given vector belongs to the space spanning other vectors. members:
      std::vector<T> coefficients - it is a vector of coefficients that
      represent the way a given vector can be assembled as a linear combination
      of other vectors in space. bool belongs_to_span - This logical variable
      indicates whether a given vector belongs to the space spanning other
      vectors.
    */
    template <typename T>
    struct is_in_span_result {
        std::vector<T> coefficients{};
        bool belongs_to_span{false};
    };

    /*
      description:
        Function is used to check whether a given vector belongs to the space
      spanning the columns of the span matrix.
    */
    template <typename T, typename LP>
    inline auto is_in_span(std::ranges::range auto v,
                           ranges::matrix_view<T, LP> span)
        -> is_in_span_result<to_fraction_type<T>>;

    /*
      description:
        The function is designed to check whether the set of vectors contained
      in the vectors matrix creates a space base.
    */
    template <typename T, typename LP>
    inline auto forms_base(ranges::matrix_view<T, LP> vectors) -> bool;

    /*
      description:
        The structure is used to represent the result of calculating the
      coordinates of a given vector relative to a given base. members:
      std::vector<T> coefficients - It is a vector of coefficients that
      represent the coordinates of a given vector relative to a given base. Each
      coefficient in this vector corresponds to one dimension in space, and its
      value determines how much a given vector contributes to a given dimension.
      bool belongs_to_base - This logical variable indicates whether a given
      vector belongs to the space stretched by a given base.
    */
    template <typename T>
    struct coordinates_in_base_result {
        std::vector<T> coefficients{};
        bool belongs_to_base{false};
    };

    /*
      description:
        Function is designed to calculate the coordinates of vector v in the
      space database defined by the base matrix columns.
    */
    template <typename T, typename LP>
    inline auto coordinates_in_base(std::ranges::range auto v,
                                    ranges::matrix_view<T, LP> base)
        -> coordinates_in_base_result<to_fraction_type<T>>;

    /*
      description:
        Function performs matrix multiplication
    */
    template <typename T, typename LP, typename LP2>
    auto matrix_multiply(ranges::matrix_view<T, LP> matrix_1,
                         ranges::matrix_view<T, LP2> matrix_2) -> matrix<T, LP>;

    /*
      description:
        Structure, which represents the result of transformation operations
      between bases in linear space. members: std::vector<T> coefficients - it
      is a vector of coefficients that represent the way a given transformation
      affects the coordinates of vectors in space. Each coefficient in this
      vector corresponds to one dimension in space, and its value determines how
      the transformation changes the contribution of a given dimension.
      ranges::matrix_view<T, LP> matrix - This variable stores a transformation
      matrix that represents the base change between two bases in linear space.
    */
    template <typename T, typename LP>
    struct base_transition_matrix_result {
        std::vector<T> coefficients;
        ranges::matrix_view<T, LP> matrix;
    };

    /*
      description:
        Function calculates the transition matrix between two linear space
      bases.
    */
    template <typename T, typename LP>
    inline auto base_transition_matrix(ranges::matrix_view<T, LP> base1,
                                       ranges::matrix_view<T, LP> base2)
        -> std::expected<algebra::base_transition_matrix_result<
                             algorithms::gaussian_elimination::fraction<T>,
                             LP>,
                         algorithms::gaussian_elimination::error>;

    /*
      description:
        Function checks whether a given vector belongs to the kernel of a given
      matrix.
    */
    template <typename T, typename LP>
    inline auto is_in_kernel(std::ranges::range auto v,
                             ranges::matrix_view<T, LP> matrix) -> bool;

    /*
      description:
        Structure, which represents the result of checking whether a given
      vector belongs to the image of a linear transformation. members: bool
      belongs_to_image - it is a logical variable that indicates whether a given
      vector belongs to a linear transformation image. std::vector<T>
      coefficients - It is a vector of coefficients that represent the way a
      given vector can be expressed as a linear combination of vectors forming
      an image of a linear transformation.
    */
    template <typename T>
    struct is_in_image_result {
        bool belongs_to_image{false};
        std::vector<T> coefficients{};
    };

    /*
      description:
        Function checks whether a given vector belongs to the matrix image and
      returns coefficients.
    */
    template <typename T, typename LP>
    inline auto is_in_image(std::ranges::range auto v,
                            ranges::matrix_view<T, LP> matrix)
        -> is_in_image_result<to_fraction_type<T>>;

}  // namespace algebra

template <typename T, typename LP>
auto algebra::first_non_zero(ranges::matrix_view<T, LP> m,
                             std::size_t row_index) -> std::size_t {
    for (std::size_t column_index = 0; column_index < m.number_of_rows();
         ++column_index) {
        if (m[row_index, column_index] != T{0}) { return column_index; }
    }
    return m.number_of_columns();
}

template <typename T, typename LP>
auto algebra::is_contradictory(ranges::matrix_view<T, LP> combined) -> bool {
    for (std::size_t row_index = 0; row_index < combined.number_of_rows();
         ++row_index) {
        if (first_non_zero(combined, row_index) ==
            (combined.number_of_columns() - 1)) {
            return true;
        }
    }
    return false;
}

template <typename T, typename LP>
auto algebra::pivots(ranges::matrix_view<T, LP> combined)
    -> std::vector<std::size_t> {
    std::vector<std::size_t> ps{};
    for (std::size_t row_index = 0; row_index < combined.number_of_rows();
         ++row_index) {
        auto pivot = first_non_zero(combined, row_index);
        if (pivot != combined.number_of_columns()) { ps.push_back(pivot); }
    }
    return ps;
}

template <typename T, typename LP>
inline auto algebra::special_solution(ranges::matrix_view<T, LP> coefficients,
                                      std::ranges::range auto y)
    -> std::optional<std::vector<algebra::to_fraction_type<T>>> {
    using namespace algorithms::gaussian_elimination;
    ranges::matrix_view y_matrix(
        y, coefficients.number_of_rows(), 1, layout::row);
    auto [combine_vector, combine_matrix] =
        combine_matrices(coefficients, y_matrix);
    auto [solution_vector, solution_matrix] =
        run(combine_matrix, reducted_form::diagonal);
    if (is_contradictory(solution_matrix)) { return std::nullopt; }
    std::vector<to_fraction_type<T>> solution{};
    solution.resize(coefficients.number_of_columns());
    auto last_column_index{solution_matrix.number_of_columns() - 1};
    for (auto [i, pivot] : std::ranges::views::zip(std::views::iota(0),
                                                   pivots(solution_matrix))) {
        solution[pivot] = solution_matrix[i, last_column_index];
    }
    return solution;
}

template <typename T, typename LP>
inline auto algebra::solve(ranges::matrix_view<T, LP> coefficients,
                           std::ranges::range auto y)
    -> algebra::solve_result<algebra::to_fraction_type<T>> {
    algebra::solve_result<algebra::to_fraction_type<T>> result;
    result.special = special_solution(coefficients, y);
    if (!result.special) { return result; }
    result.exists = true;
    result.homogeneous = kernel(coefficients);
    return result;
}

template <typename T, typename LP>
inline auto algebra::kernel_2(ranges::matrix_view<T, LP> coefficients)
    -> matrix<to_fraction_type<T>, LP> {
    using namespace algorithms::gaussian_elimination;
    auto t_coeff = ranges::transpose(coefficients);
    auto [identity_vector, identity_matrix] =
        eye<T, LP>(t_coeff.number_of_rows(), t_coeff.number_of_rows());
    auto t_identity = ranges::transpose(identity_matrix);
    auto [combined_vector, combined_matrix] =
        combine_matrices(t_coeff, t_identity);
    auto [solution_vector, solution_matrix] =
        run(combined_matrix, reducted_form::diagonal);
    return std::pair{std::move(solution_vector), solution_matrix};
}

template <typename T, typename LP>
inline auto algebra::kernel_3(ranges::matrix_view<T, LP> matrix,
                              int rows,
                              int columns,
                              int &l) -> std::vector<T> {
    std::vector<T> result;
    for (int i = 0; i < rows; i++) {
        bool helping_variable = false;
        for (int j = 0; j < columns; j++) {
            if (matrix[i, j].numerator != 0) { helping_variable = true; }
        }
        if (!helping_variable) {
            for (std::size_t j = columns; j < matrix.number_of_columns(); j++) {
                result.push_back(matrix[i, j]);
            }
            l++;
        }
    }
    return result;
}

template <typename T, typename LP>
inline auto algebra::kernel(ranges::matrix_view<T, LP> coefficients)
    -> std::pair<std::vector<to_fraction_type<T>>,
                 ::ranges::matrix_view<to_fraction_type<T>, std::layout_left>> {
    using namespace algorithms::gaussian_elimination;
    const int rows = coefficients.number_of_columns();
    const int columns = coefficients.number_of_rows();
    auto [solution_vector, solution_matrix] = kernel_2(coefficients);
    int result_rows = 0;
    auto result = kernel_3(solution_matrix, rows, columns, result_rows);
    if (result.size() != 0) {
        ranges::matrix_view result_m(
            result, result_rows, result.size() / result_rows, layout::column);
        return std::pair{std::move(result), result_m};
    }
    result.push_back(to_fraction_type<T>{0});
    ranges::matrix_view result_m(result, 1, 1, layout::column);
    return std::pair{std::move(result), result_m};
}

template <typename T, typename LP>
inline auto algebra::is_in_span(std::ranges::range auto v,
                                ranges::matrix_view<T, LP> span)
    -> algebra::is_in_span_result<to_fraction_type<T>> {
    algebra::is_in_span_result<to_fraction_type<T>> result;
    auto result_solve = algebra::solve(span, v);
    if (!result_solve.exists) { return result; }
    if (result_solve.special) {
        result.belongs_to_span = true;
        result.coefficients = result_solve.special.value();
    }
    return result;
}

template <typename T, typename LP>
inline auto algebra::forms_base(ranges::matrix_view<T, LP> vectors) -> bool {
    int rows = vectors.number_of_rows();
    int columns = vectors.number_of_columns();
    if (rows == columns) {
        auto det =
            algorithms::gaussian_elimination::determinant(vectors).value();
        if (det.numerator != 0) { return true; }
    }
    return false;
}

template <typename T, typename LP>
inline auto algebra::coordinates_in_base(std::ranges::range auto v,
                                         ranges::matrix_view<T, LP> base)
    -> algebra::coordinates_in_base_result<to_fraction_type<T>> {
    algebra::coordinates_in_base_result<to_fraction_type<T>> result{};
    if (!algebra::forms_base(base)) { return result; }
    auto result_of_span = algebra::is_in_span(v, base);
    if (result_of_span.belongs_to_span) {
        result.belongs_to_base = true;
        result.coefficients = result_of_span.coefficients;
    }
    return result;
}

template <typename T, typename LP, typename LP2>
auto algebra::matrix_multiply(ranges::matrix_view<T, LP> matrix_1,
                              ranges::matrix_view<T, LP2> matrix_2)
    -> matrix<T, LP> {
    if (matrix_1.number_of_columns() != matrix_2.number_of_rows()) {
        throw std::invalid_argument(
            "\nMatrices are not compalible for multiplication");
    }
    auto result_rows = matrix_1.number_of_rows();
    auto result_columns = matrix_2.number_of_columns();
    std::vector<T> result_vector(result_rows * result_columns);
    ranges::matrix_view result(
        result_vector, result_rows, result_columns, layout::row);
    for (std::size_t i = 0; i < result_rows; ++i) {
        for (std::size_t j = 0; j < result_columns; ++j) {
            T sum{0};
            for (std::size_t k = 0; k < matrix_1.number_of_columns(); ++k) {
                sum += matrix_1[i, k] * matrix_2[k, j];
            }
            result[i, j] = sum;
        }
    }
    return std::pair{std::move(result_vector), result};
}

template <typename T, typename LP>
inline auto algebra::base_transition_matrix(ranges::matrix_view<T, LP> base1,
                                            ranges::matrix_view<T, LP> base2)
    -> std::expected<algebra::base_transition_matrix_result<
                         algorithms::gaussian_elimination::fraction<T>,
                         LP>,
                     algorithms::gaussian_elimination::error> {
    auto inv = algorithms::gaussian_elimination::inverse(base2);
    if (!inv) {
        std::print("Errror:{}", std::to_underlying(inv.error()));
        return std::unexpected{inv.error()};
    }
    auto [inverse_vector, inverse_matrix] = inv.value();
    auto [new_base1_v, new_base1_m] =
        algorithms::gaussian_elimination::convert_to_matrix_of_fractions(base1);
    auto [result_vector, result_matrix] =
        algebra::matrix_multiply(inverse_matrix, new_base1_m);
    return algebra::base_transition_matrix_result{result_vector, result_matrix};
}

template <typename T, typename LP>
inline auto algebra::is_in_kernel(std::ranges::range auto v,
                                  ranges::matrix_view<T, LP> matrix) -> bool {
    ranges::matrix_view v_matrix(v, v.size(), 1, layout::row);
    if (v.size() != matrix.number_of_columns()) { return false; }
    auto [result_vector, result_matrix] =
        algebra::matrix_multiply(matrix, v_matrix);
    for (std::size_t i = 0; i < result_matrix.number_of_columns(); i++) {
        if (result_matrix[i, 0] != 0) { return false; }
    }
    return true;
}

template <typename T, typename LP>
inline auto algebra::is_in_image(std::ranges::range auto v,
                                 ranges::matrix_view<T, LP> matrix)
    -> algebra::is_in_image_result<to_fraction_type<T>> {
    algebra::is_in_image_result<to_fraction_type<T>> result;
    auto result_solve = algebra::solve(matrix, v);
    if (!result_solve.exists) { return result; }
    if (result_solve.special) {
        result.belongs_to_image = true;
        result.coefficients = result_solve.special.value();
    }
    return result;
}

/*
  Tests
*/
namespace tests_of_algebra {

    auto test_of_solve()->bool {
        std::vector v1{1, 0, 0, 0, 2, 0, 0, 0, 7, 1, 2, 3};
        std::vector v2{1, 2, 3};
        ::ranges::matrix_view m(v1, 3, 4, layout::row);
        auto result = algebra::solve(m, v2);
        assert(static_cast<bool>(result.exists));
        assert(result.special.has_value());
        auto special_solution = result.special.value();
        assert(special_solution.size() == 4);

        return true;
    }

    auto test_of_is_in_span()->bool {
        std::vector v1{1, 2, 3, 4, 5, 6, 7, 8, 9};
        std::vector v2{1, 2, 3};
        ::ranges::matrix_view m(v1, 3, 3, layout::row);
        auto result = algebra::is_in_span(v2, m);
        assert(static_cast<bool>(result.belongs_to_span));
        assert(result.coefficients.size() == 3);

        return true;
    }

    auto test_of_forms_base()->bool {
        std::vector v{1, 0, 0, 0, 5, 0, 0, 0, 9};
        ::ranges::matrix_view m(v, 3, 3, layout::row);
        bool result = algebra::forms_base(m);
        assert(result);

        return true;
    }

    auto test_of_coordinates_in_base()->bool {
        std::vector v1{1, 2, 1, 5, 6, 3, 9, 1, 7};
        std::vector v2{1, 2, 3};
        ::ranges::matrix_view m(v1, 3, 3, layout::row);
        auto result = algebra::coordinates_in_base(v2, m);
        assert(static_cast<bool>(result.belongs_to_base));
        assert(result.coefficients.size() == 3);

        return true;
    }

    auto test_of_base_transition_matrix()->bool {
        std::vector v1{1, 6, 3, 7, 0, 4, 8, 2, 1};
        std::vector v2{9, 5, 1, 7, 4, 6, 9, 1, 6};
        ::ranges::matrix_view m1(v1, 3, 3, layout::row);
        ::ranges::matrix_view m2(v2, 3, 3, layout::row);
        auto result = algebra::base_transition_matrix(m1, m2);
        assert(result.has_value());

        auto [result_vector, result_matrix] = *result;
        assert(result_matrix.number_of_rows() == 3);
        assert(result_matrix.number_of_columns() == 3);

        return true;
    }

    auto test_of_is_in_kernel()->bool {
        std::vector v1{1, 2, 5, 8, 4, 2, 9, 0, 3};
        std::vector v2{1, 7, 4};
        ::ranges::matrix_view m(v1, 3, 3, layout::row);
        bool result = algebra::is_in_kernel(v2, m);
        assert(!result);

        return true;
    }

    auto test_of_is_in_image()->bool {
        std::vector v1{1, 2, 6, 9, 1, 5, 9, 5, 11};
        std::vector v2{5, 8, 4};
        ::ranges::matrix_view m(v1, 3, 3, layout::row);
        auto result = algebra::is_in_image(v2, m);
        assert(static_cast<bool>(result.belongs_to_image));
        assert(result.coefficients.size() == 3);

        return true;
    }

    void all_test() {
        assert(test_of_solve());
        assert(test_of_is_in_span());
        assert(test_of_forms_base());
        assert(test_of_coordinates_in_base());
        assert(test_of_base_transition_matrix());
        assert(test_of_is_in_kernel());
        assert(test_of_is_in_image());
        std::println("\n\nAll Test Passed Succesfully!");
    }
}  // namespace tests_of_algebra

/*
  Examples of using algebra namespace
*/
namespace examples_of_algebra {

    auto example_of_solve() -> void {
        std::println("\nExample of solve:\n");
        std::vector v1 = {1, 2, 6, 3, 9, 5, 0, 5, 1};
        std::vector v2 = {6, 8, 5};
        ::ranges::matrix_view m(v1, 3, 3, layout::row);
        auto result = algebra::solve(m, v2);
        if (result.special) {
            std::println("exists: {}", result.exists);
            std::println("special: {}", result.special.value());
        }
        if (result.homogeneous) {
            auto [vector, matrix] = result.homogeneous.value();
            std::println("homogeneous: {}", matrix);
        }
    }

    auto example_of_is_in_span() {
        std::println("\nExample of is_in_span");
        std::vector v1 = {1, 2, 6, 3, 9, 5, 0, 5, 1};
        std::vector v2 = {1, 8, 7};
        ::ranges::matrix_view m(v1, 3, 3, layout::row);
        auto result = algebra::is_in_span(v2, m);
        if (result.belongs_to_span) {
            std::println("cpefficients: {}", result.coefficients);
        }
    }

    auto example_of_coordinates_in_base() -> void {
        std::println("\nExample of coordinates_in_base");
        std::vector v1 = {1, 3, 6, 3, 0, 5, 2, 3, 7};
        std::vector v2 = {1, 6, 4};
        ::ranges::matrix_view m(v1, 3, 3, layout::row);
        auto result = algebra::coordinates_in_base(v2, m);
        if (result.belongs_to_base) {
            std::println("coefficients: {}", result.coefficients);
        }
    }

    auto example_of_base_transition_matrix() -> void {
        std::println("\nExample of base_transition_matrix");
        std::vector v1{1, 2, 3, 4};
        std::vector v2{2, 3, 1, 4};
        ::ranges::matrix_view m1(v1, 2, 2, layout::row);
        ::ranges::matrix_view m2(v2, 2, 2, layout::row);
        auto result = algebra::base_transition_matrix(m2, m1);
        if (result) { std::println("matrix: {}", result->coefficients); }
    }

    auto example_of_is_in_image() -> void {
        std::println("\nExample of is_in_image");
        std::vector v1{6, 8, 3, 9, 2, 7, 4, 9, 5};
        std::vector v2{6, 4, 7};
        ::ranges::matrix_view m(v1, 3, 3, layout::row);
        auto result = algebra::is_in_image(v2, m);
        if (result.belongs_to_image) {
            std::println("coefficients: {}", result.coefficients);
        }
    }

    auto examples() -> void {
        std::println("\n\nExamples:");
        example_of_solve();
        example_of_is_in_span();
        example_of_coordinates_in_base();
        example_of_base_transition_matrix();
        example_of_is_in_image();
    }

}  // namespace examples_of_algebra