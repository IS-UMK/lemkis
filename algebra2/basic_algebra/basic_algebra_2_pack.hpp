#pragma once
#include "gaussian_elimination.hpp"

namespace algebra {

template <typename T, typename Layout>
using matrix = std::pair<std::vector<T>, ranges::matrix_view<T, Layout>>;

template <typename T>
using homogeneous_solution_t = std::optional<matrix<T, std::layout_left>>;

template <typename T>
using to_fraction_type =
    std::conditional_t<algorithms::gaussian_elimination::Fraction<T>, T,
                       algorithms::gaussian_elimination::fraction<T>>;

template <typename T, typename LP>
auto first_non_zero(ranges::matrix_view<T, LP> m, std::size_t row_index)
    -> std::size_t;

template <typename T, typename LP>
auto is_contradictory(ranges::matrix_view<T, LP> combined) -> bool;

template <typename T, typename LP>
auto pivots(ranges::matrix_view<T, LP> combined) -> std::vector<std::size_t>;

template <typename T, typename LP>
inline auto special_solution(ranges::matrix_view<T, LP> coefficients,
                             std::ranges::range auto y)
    -> std::optional<std::vector<to_fraction_type<T>>>;

template <typename T> struct solve_result {
  bool exists{false};
  std::optional<std::vector<T>> special{};
  homogeneous_solution_t<T> homogeneous{};
};

template <typename T, typename LP>
inline auto solve(ranges::matrix_view<T, LP> coefficients,
                  std::ranges::range auto y)
    -> solve_result<to_fraction_type<T>>;

template <typename T, typename LP>
inline auto kernel(ranges::matrix_view<T, LP> coefficients)
    -> std::pair<std::vector<to_fraction_type<T>>,
                 ::ranges::matrix_view<to_fraction_type<T>, std::layout_left>>;

template <typename T> struct is_in_span_result {
  std::vector<T> coefficients{};
  bool belongs_to_span{false};
};

template <typename T, typename LP>
inline auto is_in_span(std::ranges::range auto v,
                       ranges::matrix_view<T, LP> span)
    -> is_in_span_result<to_fraction_type<T>>;

template <typename T, typename LP>
inline auto forms_base(ranges::matrix_view<T, LP> vectors) -> bool;

template <typename T> struct coordinates_in_base_result {
  std::vector<T> coefficients{};
  bool belongs_to_base{false};
};

template <typename T, typename LP>
inline auto coordinates_in_base(std::ranges::range auto v,
                                ranges::matrix_view<T, LP> base)
    -> coordinates_in_base_result<to_fraction_type<T>>;

template <typename T, typename LP, typename LP2>
auto matrix_multiply(ranges::matrix_view<T, LP> matrix_1,
                     ranges::matrix_view<T, LP2> matrix_2) -> matrix<T, LP>;

template <typename T, typename LP> struct base_transition_matrix_result {
  std::vector<T> coefficients;
  ranges::matrix_view<T, LP> matrix;
};

template <typename T, typename LP>
inline auto base_transition_matrix(ranges::matrix_view<T, LP> base1,
                                   ranges::matrix_view<T, LP> base2)
    -> std::expected<algebra::base_transition_matrix_result<
                         algorithms::gaussian_elimination::fraction<T>, LP>,
                     algorithms::gaussian_elimination::error>;

template <typename T, typename LP>
inline auto is_in_kernel(std::ranges::range auto v,
                         ranges::matrix_view<T, LP> matrix) -> bool;

template <typename T> struct is_in_image_result {
  bool belongs_to_image{false};
  std::vector<T> coefficients{};
};

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
inline auto algebra::kernel(ranges::matrix_view<T, LP> coefficients)
    -> std::pair<std::vector<to_fraction_type<T>>,
                 ::ranges::matrix_view<to_fraction_type<T>, std::layout_left>> {
  using namespace algorithms::gaussian_elimination;
  auto t_coeff = ranges::transpose(coefficients);
  int rows = t_coeff.extent(0);
  int columns = t_coeff.extent(1);
  bool helping_variable;
  std::vector<to_fraction_type<T>> result;
  auto [identity_vector, identity_matrix] = eye<T, LP>(rows, rows);
  auto [combined_vector, combined_matrix] =
      combine_matrices(t_coeff, identity_matrix);
  auto [solution_vector, solution_matrix] =
      run(combined_matrix, axis::rows, reducted_form::diagonal);
  int columns_2 = solution_matrix.extent(1);
  int l = 0;
  for (int i = 0; i < rows; i++) {
    helping_variable = false;
    for (int j = 0; j < columns; j++) {
      if (solution_matrix[i, j].numerator != 0) {
        helping_variable = true;
      }
    }
    if (helping_variable == false) {
      for (int j = columns; j < columns_2; j++) {
        result.push_back(solution_matrix[i, j]);
      }
      l++;
    }
  }
  if (result.size() != 0) {
    ranges::matrix_view result_m(result, l, result.size() / l, layout::column);
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