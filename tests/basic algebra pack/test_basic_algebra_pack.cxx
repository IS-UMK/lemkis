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

import matrix;
import expect;
import basic_algebra_pack;
import gaussian elimination;

bool test_of_solve() {
  std::vector v1{1, 0, 0, 0, 2, 0, 0, 0, 7, 1, 2, 3};
  std::vector v2{1, 2, 3};
  ::ranges::matrix_view m(v1, 3, 4, layout::row);
  auto result = algebra::solve(m, v2);
  assert(result.exists == true);
  assert(result.special.has_value());
  auto special_solution = result.special.value();
  assert(special_solution.size() == 4);

  return true;
}

bool test_of_is_in_span() {
  std::vector v1{1, 2, 3, 4, 5, 6, 7, 8, 9};
  std::vector v2{1, 2, 3};
  ::ranges::matrix_view m(v1, 3, 3, layout::row);
  auto result = algebra::is_in_span(v2, m);
  assert(result.belongs_to_span == true);
  assert(result.coefficients.size() == 3);

  return true;
}

bool test_of_forms_base() {
  std::vector v{1, 0, 0, 0, 5, 0, 0, 0, 9};
  ::ranges::matrix_view m(v, 3, 3, layout::row);
  bool result = algebra::forms_base(m);
  assert(result == true);

  return true;
}

bool test_of_coordinates_in_base() {
  std::vector v1{1, 2, 1, 5, 6, 3, 9, 1, 7};
  std::vector v2{1, 2, 3};
  ::ranges::matrix_view m(v1, 3, 3, layout::row);
  auto result = algebra::coordinates_in_base(v2, m);
  assert(result.belongs_to_base == true);
  assert(result.coefficients.size() == 3);

  return true;
}

bool test_of_base_transition_matrix() {
  std::vector v1{1, 6, 3, 7, 0, 4, 8, 2, 1};
  std::vector v2{9, 5, 1, 7, 4, 6, 9, 1, 6};
  ::ranges::matrix_view m1(v1, 3, 3, layout::row);
  ::ranges::matrix_view m2(v2, 3, 3, layout::row);
  auto result = algebra::base_transition_matrix(m1, m2);
  assert(result.has_value() == true);

  auto [result_vector, result_matrix] = *result;
  assert(result_matrix.number_of_rows() == 3);
  assert(result_matrix.number_of_columns() == 3);

  return true;
}

bool test_of_is_in_kernel() {
  std::vector v1{1, 2, 5, 8, 4, 2, 9, 0, 3};
  std::vector v2{1, 7, 4};
  ::ranges::matrix_view m(v1, 3, 3, layout::row);
  bool result = algebra::is_in_kernel(v2, m);
  assert(result == false);

  return true;
}

bool test_of_is_in_image() {
  std::vector v1{1, 2, 6, 9, 1, 5, 9, 5, 11};
  std::vector v2{5, 8, 4};
  ::ranges::matrix_view m(v1, 3, 3, layout::row);
  auto result = algebra::is_in_image(v2, m);
  assert(result.belongs_to_image == true);
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
} // namespace tests_of_algebra

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
  if (result) {
    std::println("matrix: {}", result->coefficients);
  }
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
