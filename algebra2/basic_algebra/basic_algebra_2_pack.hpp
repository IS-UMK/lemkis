#pragma once
#include "gaussian_elimination.hpp"

namespace algebra {

/*
  description:
    Alias type for matrix with data vector and matrix view
*/
template <typename T, typename Layout>
using matrix = std::pair<std::vector<T>, ranges::matrix_view<T, Layout>>;


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
    std::conditional_t<algorithms::gaussian_elimination::Fraction<T>, T,
                       algorithms::gaussian_elimination::fraction<T>>;


/*
  description:
    The function is used to find the first non-zero element in a specific row of
  the matrix
*/
template <typename T, typename LP>
auto first_non_zero(ranges::matrix_view<T, LP> m, std::size_t row_index)
    -> std::size_t;


/*
  description:
    The function checks whether there is a row in the given matrix that can be
  considered contradictory.
*/
template <typename T, typename LP>
auto is_contradictory(ranges::matrix_view<T, LP> combined) -> bool;


/*
  description:
    The function it is used to find the indexes of the first non-zero elements
  (pivots) in each row of the matrix and return them as a vector.
*/
template <typename T, typename LP>
auto pivots(ranges::matrix_view<T, LP> combined) -> std::vector<std::size_t>;


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
    std::optional<std::vector<T>> special - If the solution operation generates
  special results, such as special values, they will be stored in this vector
    homogeneous_solution_t<T> homogeneous - This variable stores a homogeneous
  solution of the operation that was performed.
*/
template <typename T> struct solve_result {
  bool exists{false};
  std::optional<std::vector<T>> special{};
  homogeneous_solution_t<T> homogeneous{};
};


/*
  description:
    A function that is designed to solve systems of linear equations in the form
  of Ax = y.
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
inline auto kernel_3(ranges::matrix_view<T, LP> matrix, int rows, int columns,
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
    -> std::pair<std::vector<to_fraction_type<T>>,
                 ::ranges::matrix_view<to_fraction_type<T>, std::layout_left>>;


/*
  description:
    The structure is used to represent the result of checking whether a given
  vector belongs to the space spanning other vectors. members: std::vector<T>
  coefficients - it is a vector of coefficients that represent the way a given
  vector can be assembled as a linear combination of other vectors in space.
    bool belongs_to_span - This logical variable indicates whether a given
  vector belongs to the space spanning other vectors.
*/
template <typename T> struct is_in_span_result {
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
    The function is designed to check whether the set of vectors contained in
  the vectors matrix creates a space base.
*/
template <typename T, typename LP>
inline auto forms_base(ranges::matrix_view<T, LP> vectors) -> bool;


/*
  description:
    The structure is used to represent the result of calculating the coordinates
  of a given vector relative to a given base. members: std::vector<T>
  coefficients - It is a vector of coefficients that represent the coordinates
  of a given vector relative to a given base. Each coefficient in this vector
  corresponds to one dimension in space, and its value determines how much a
  given vector contributes to a given dimension. bool belongs_to_base - This
  logical variable indicates whether a given vector belongs to the space
  stretched by a given base.
*/
template <typename T> struct coordinates_in_base_result {
  std::vector<T> coefficients{};
  bool belongs_to_base{false};
};


/*
  description:
    Function is designed to calculate the coordinates of vector v in the space
  database defined by the base matrix columns.
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
    Structure, which represents the result of transformation operations between
  bases in linear space. members: std::vector<T> coefficients - it is a vector
  of coefficients that represent the way a given transformation affects the
  coordinates of vectors in space. Each coefficient in this vector corresponds
  to one dimension in space, and its value determines how the transformation
  changes the contribution of a given dimension. ranges::matrix_view<T, LP>
  matrix - This variable stores a transformation matrix that represents the base
  change between two bases in linear space.
*/
template <typename T, typename LP> struct base_transition_matrix_result {
  std::vector<T> coefficients;
  ranges::matrix_view<T, LP> matrix;
};


/*
  description:
    Function calculates the transition matrix between two linear space bases.
*/
template <typename T, typename LP>
inline auto base_transition_matrix(ranges::matrix_view<T, LP> base1,
                                   ranges::matrix_view<T, LP> base2)
    -> std::expected<algebra::base_transition_matrix_result<
                         algorithms::gaussian_elimination::fraction<T>, LP>,
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
    Structure, which represents the result of checking whether a given vector
  belongs to the image of a linear transformation. members: bool
  belongs_to_image - it is a logical variable that indicates whether a given
  vector belongs to a linear transformation image. std::vector<T> coefficients -
  It is a vector of coefficients that represent the way a given vector can be
  expressed as a linear combination of vectors forming an image of a linear
  transformation.
*/
template <typename T> struct is_in_image_result {
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


} // namespace algebra

template <typename T, typename LP>
auto algebra::first_non_zero(ranges::matrix_view<T, LP> m,
                             std::size_t row_index) -> std::size_t {
  for (std::size_t column_index = 0; column_index < m.extent(0);
       ++column_index) {
    if (m[row_index, column_index] != T{0}) {
      return column_index;
    }
  }
  return m.extent(1);
}


template <typename T, typename LP>
auto algebra::is_contradictory(ranges::matrix_view<T, LP> combined) -> bool {
  for (std::size_t row_index = 0; row_index < combined.extent(0); ++row_index) {
    if (first_non_zero(combined, row_index) == (combined.extent(1) - 1)) {
      return true;
    }
  }
  return false;
}


template <typename T, typename LP>
auto algebra::pivots(ranges::matrix_view<T, LP> combined)
    -> std::vector<std::size_t> {
  std::vector<std::size_t> ps{};
  for (std::size_t row_index = 0; row_index < combined.extent(0); ++row_index) {
    auto pivot = first_non_zero(combined, row_index);
    if (pivot != combined.extent(1)) {
      ps.push_back(pivot);
    }
  }
  return ps;
}


template <typename T, typename LP>
inline auto algebra::special_solution(ranges::matrix_view<T, LP> coefficients,
                                      std::ranges::range auto y)
    -> std::optional<std::vector<algebra::to_fraction_type<T>>> {
  using namespace algorithms::gaussian_elimination;
  ranges::matrix_view y_matrix(y, coefficients.extent(0), 1, layout::column);
  auto [combine_vector, combine_matrix] =
      combine_matrices(coefficients, y_matrix);
  auto [solution_vector, solution_matrix] =
      run(combine_matrix, axis::rows, reducted_form::diagonal);
  if (is_contradictory(solution_matrix)) {
    return std::nullopt;
  }
  std::vector<to_fraction_type<T>> solution{};
  solution.resize(coefficients.extent(1));
  auto last_column_index{solution_matrix.extent(1) - 1};
  for (auto [i, pivot] :
       std::ranges::views::zip(std::views::iota(0), pivots(solution_matrix))) {
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
  if (!result.special) {
    return result;
  }
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
      eye<T, LP>(t_coeff.extent(0), t_coeff.extent(0));
  auto [combined_vector, combined_matrix] =
      combine_matrices(t_coeff, identity_matrix);
  auto [solution_vector, solution_matrix] =
      run(combined_matrix, axis::rows, reducted_form::diagonal);
  return std::pair{std::move(solution_vector), solution_matrix};
}


template <typename T, typename LP>
inline auto algebra::kernel_3(ranges::matrix_view<T, LP> matrix, int rows,
                              int columns, int &l) -> std::vector<T> {
  std::vector<T> result;
  for (int i = 0; i < rows; i++) {
    bool helping_variable = false;
    for (int j = 0; j < columns; j++) {
      if (matrix[i, j].numerator != 0) {
        helping_variable = true;
      }
    }
    if (!helping_variable) {
      for (int j = columns; j < matrix.extent(1); j++) {
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
  int rows = coefficients.extent(1);
  int columns = coefficients.extent(0);
  auto [solution_vector, solution_matrix] = kernel_2(coefficients);
  int result_rows = 0;
  auto result = kernel_3(solution_matrix, rows, columns, result_rows);
  if (result.size() != 0) {
    ranges::matrix_view result_m(result, result_rows,
                                 result.size() / result_rows, layout::column);
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
  if (!result_solve.exists) {
    return result;
  }
  if (result_solve.special) {
    result.belongs_to_span = true;
    result.coefficients = result_solve.special.value();
  }
  return result;
}


template <typename T, typename LP>
inline auto algebra::forms_base(ranges::matrix_view<T, LP> vectors) -> bool {
  int rows = vectors.extent(0);
  int columns = vectors.extent(1);
  if (rows == columns) {
    auto det = algorithms::gaussian_elimination::determinant(vectors).value();
    if (det.numerator != 0) {
      return true;
    }
  }
  return false;
}


template <typename T, typename LP>
inline auto algebra::coordinates_in_base(std::ranges::range auto v,
                                         ranges::matrix_view<T, LP> base)
    -> algebra::coordinates_in_base_result<to_fraction_type<T>> {
  algebra::coordinates_in_base_result<to_fraction_type<T>> result{};
  if (!algebra::forms_base(base)) {
    return result;
  }
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
  if (matrix_1.extent(1) != matrix_2.extent(0)) {
    throw std::invalid_argument(
        "\nMatrices are not compalible for multiplication");
  }
  auto result_rows = matrix_1.extent(0);
  auto result_columns = matrix_2.extent(1);
  std::vector<T> result_vector(result_rows * result_columns);
  ranges::matrix_view result(result_vector, result_rows, result_columns,
                             layout::row);
  for (std::size_t i = 0; i < result_rows; ++i) {
    for (std::size_t j = 0; j < result_columns; ++j) {
      T sum{0};
      for (std::size_t k = 0; k < matrix_1.extent(1); ++k) {
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
                         algorithms::gaussian_elimination::fraction<T>, LP>,
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
  if (v.size() != matrix.extent(1)) {
    return false;
  }
  auto [result_vector, result_matrix] =
      algebra::matrix_multiply(matrix, v_matrix);
  for (int i = 0; i < result_matrix.extent(1); i++) {
    if (result_matrix[i, 0] != 0) {
      return false;
    }
  }
  return true;
}


template <typename T, typename LP>
inline auto algebra::is_in_image(std::ranges::range auto v,
                                 ranges::matrix_view<T, LP> matrix)
    -> algebra::is_in_image_result<to_fraction_type<T>> {
  algebra::is_in_image_result<to_fraction_type<T>> result;
  auto result_solve = algebra::solve(matrix, v);
  if (!result_solve.exists) {
    return result;
  }
  if (result_solve.special) {
    result.belongs_to_image = true;
    result.coefficients = result_solve.special.value();
  }
  return result;
}


/*
  Examples of using
*/
auto test_of_solve() -> void {
  std::vector v1{1, 0, 0, 0, 2, 0, 0, 0, 7, 1, 2, 3};
  std::vector v2{1, 2, 3};
  ::ranges::matrix_view m(v1, 3, 4, layout::row);
  auto result = algebra::solve(m, v2);
  std::println("exists: {}", result.exists);
  if (result.special) {
    std::println("special = {}", result.special.value());
  }
  if (result.homogeneous) {
    auto [vector, matrix] = result.homogeneous.value();
    std::println("m = {}", matrix);
  }
}


auto test_of_is_in_span() -> void {
  std::vector v1{1, 2, 3, 4, 5, 6, 7, 8, 9};
  std::vector v2{1, 2, 3};
  ::ranges::matrix_view m(v1, 3, 3, layout::row);
  auto result = algebra::is_in_span(v2, m);
  if (result.belongs_to_span) {
    std::println("Coefficients = {}", result.coefficients);
  }
}


auto test_of_forms_base() -> void {
  std::vector v{1, 0, 0, 0, 5, 0, 0, 0, 9};
  ::ranges::matrix_view m(v, 3, 3, layout::row);
  std::println("Form base = {}", algebra::forms_base(m));
}


auto test_of_coordinates_in_base() -> void {
  std::vector v1{1, 2, 1, 5, 6, 3, 9, 1, 7};
  std::vector v2{1, 2, 3};
  ::ranges::matrix_view m(v1, 3, 3, layout::row);
  auto result = algebra::coordinates_in_base(v2, m);
  if (result.belongs_to_base) {
    std::println("coefficients = {}", result.coefficients);
  }
}


auto test_of_base_transition_matrix() -> void {
  std::vector v1{1, 6, 3, 7, 0, 4, 8, 2, 1};
  std::vector v2{9, 5, 1, 7, 4, 6, 9, 1, 6};
  ::ranges::matrix_view m1(v1, 3, 3, layout::row);
  ::ranges::matrix_view m2(v2, 3, 3, layout::row);
  auto result = algebra::base_transition_matrix(m1, m2);
  if (!result) {
    std::print("Errror:{}", std::to_underlying(result.error()));
  } else {
    auto [result_vector, result_matrix] = *result;
    std::println("base = {}", result_matrix);
  }
}


auto test_of_is_in_kernel() -> void {
  std::vector v1{1, 2, 5, 8, 4, 2, 9, 0, 3};
  std::vector v2{1, 7, 4};
  ::ranges::matrix_view m(v1, 3, 3, layout::row);
  std::println("is in kernel = {}", algebra::is_in_kernel(v2, m));
}


auto test_of_is_in_image() -> void {
  std::vector v1{1, 2, 6, 9, 1, 5, 9, 5, 11};
  std::vector v2{5, 8, 4};
  ::ranges::matrix_view m(v1, 3, 3, layout::row);
  auto result = algebra::is_in_image(v2, m);
  if (result.belongs_to_image) {
    std::println("coefficients = {}", result.coefficients);
  }
}


auto all_tests() -> void {
  std::println("\n\nTEST OF SOLVE:");
  test_of_solve();
  std::println("\n\nTEST OF IS_IN_SPAN:");
  test_of_is_in_span();
  std::println("\n\nTEST OF FORMS_BASE:");
  test_of_forms_base();
  std::println("\n\nTEST OF COORDINATES IN BASE:");
  test_of_coordinates_in_base();
  std::println("\n\nTEST OF BASE TRANSITION_MATRIX:");
  test_of_base_transition_matrix();
  std::println("\n\nTEST OF IS IN KERNEL:");
  test_of_is_in_kernel();
  std::println("\n\nTEST OF IS IN IMAGE:");
  test_of_is_in_image();
}