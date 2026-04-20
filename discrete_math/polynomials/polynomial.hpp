#pragma once
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <format>
#include <iostream>
#include <numeric>
#include <print>
#include <set>
#include <stdexcept>
#include <utility>
#include <vector>

namespace polynomial {
template <typename T>
struct polynomial {
 public:
  std::vector<T> coefficients{};
  std::size_t degree{};

 public:
  /*
     description:
         Adds another polynomial to this polynomial.
     parameters:
         p - the polynomial to be added
     return:
         polynomial& - the result of the addition
 */
  auto operator+=(const polynomial &p) -> polynomial & {
    if (p.degree > degree) {
      coefficients.resize(p.degree + 1, T{0});
    }
    for (std::size_t i = 0; i <= p.degree; ++i) {
      coefficients[i] += p.coefficients[i];
      if (coefficients[i] != 0) {
        degree = i;
      }
    }
    return *this;
  }
  /*
      description:
          Negates the polynomial.
      return:
          polynomial - the negated polynomial
  */
  auto operator-() const {
    auto copy{*this};
    for (auto &c : copy.coefficients) {
      c *= -1;
    }
    return copy;
  }
  /*
      description:
          Subtracts another polynomial from this polynomial.
      parameters:
          p - the polynomial to be subtracted
      return:
          polynomial& - the result of the subtraction
  */
  auto operator-=(const polynomial &p) -> polynomial & {
    return operator+=(-p);
  }
  /*
      description:
          Multiplies this polynomial by another polynomial.
      parameters:
          p - the polynomial to be multiplied
      return:
          polynomial& - the result of the multiplication
  */
  auto operator*=(const polynomial &p) -> polynomial & {
    std::vector<T> multiplication_vector{};
    multiplication_vector.resize(p.degree + degree + 1, T{0});
    coefficients.resize(p.degree + degree + 1, T{0});
    for (std::size_t i = degree + 1; i > 0; i--) {
      for (std::size_t j = p.degree + 1; j > 0; j--) {
        multiplication_vector[i + j - 2] +=
            coefficients[i - 1] * p.coefficients[j - 1];
      }
    }
    coefficients = multiplication_vector;
    degree += p.degree;
    return *this;
  }
  /*
      description:
          Divides this polynomial by another polynomial.
      parameters:
          p - the polynomial to divide by
      return:
          polynomial& - the result of the division
  */
  auto operator/=(const polynomial &p) -> polynomial & {
    if ((p.degree > degree) || (p.degree == 0)) {
      throw std::invalid_argument("Division by invalid polynomial");
    }
    std::vector<T> quo(degree - p.degree + 1, T{0});
    for (std::size_t i = degree; i > p.degree - 1; i--) {
      quo[i - p.degree] = coefficients[i] / p.coefficients[p.degree];
      for (std::size_t j = 0; j < p.degree + 1; j++) {
        coefficients[i - j] -= quo[i - p.degree] * p.coefficients[p.degree - j];
      }
    }
    degree -= p.degree;
    coefficients = quo;
    return *this;
  }
  /*
      description:
          Computes the remainder of this polynomial divided by another
     polynomial. parameters: p - the polynomial to divide by return:
     polynomial&
     - the remainder polynomial
  */
  auto operator%=(const polynomial &p) -> polynomial & {
    if (p.degree == 0 && p.coefficients[0] == T{0}) {
      throw std::invalid_argument("Division by zero polynomial");
    }
    *this -= (*this / p) * p;
    return *this;
  }
  /*
      description:
          Evaluates the polynomial at a given value.
      parameters:
          value - the value at which to evaluate the polynomial
      return:
          T - the result of the evaluation
  */
 public:
  auto operator()(T value) {
    T result = 0;
    for (int i = this->degree; i >= 0; --i) {
      result = result * value + this->coefficients[i];
    }
    return result;
  }
};
/*
    description:
        Adds two polynomials.
    parameters:
        p - the first polynomial
        q - the second polynomial
    return:
        polynomial<T> - the result of the addition
*/
template <typename T>
inline auto operator+(polynomial<T> p, polynomial<T> q) {
  p += q;
  return p;
}
/*
    description:
        Subtracts the second polynomial from the first polynomial.
    parameters:
        p - the first polynomial
        q - the second polynomial
    return:
        polynomial<T> - the result of the subtraction
*/
template <typename T>
inline auto operator-(polynomial<T> p, polynomial<T> q) {
  p -= q;
  return p;
}
/*
    description:
        Multiplies two polynomials.
    parameters:
        p - the first polynomial
        q - the second polynomial
    return:
        polynomial<T> - the result of the multiplication
*/
template <typename T>
inline auto operator*(polynomial<T> p, polynomial<T> q) {
  p *= q;
  return p;
}
/*
    description:
        Divides the first polynomial by the second polynomial.
    parameters:
        p - the first polynomial
        q - the second polynomial
    return:
        polynomial<T> - the result of the division
*/
template <typename T>
inline auto operator/(polynomial<T> p, polynomial<T> q) {
  p /= q;
  return p;
}
/*
    description:
        Computes the remainder of the first polynomial divided by the second
   polynomial. parameters: p - the first polynomial q - the second
   polynomial return: polynomial<T> - the remainder polynomial
*/
template <typename T>
inline auto operator%(polynomial<T> p, polynomial<T> q) {
  p %= q;
  return p;
}
/*
    description:
        Divides the first polynomial by the second polynomial and returns
   both the quotient and the remainder. parameters:
   p - the first polynomial
   q - the second polynomial return: std::pair<polynomial<T>, polynomial<T>>
   - a pair containing the quotient and the remainder
*/
template <typename T>
inline auto divide(polynomial<T> p, polynomial<T> q)
    -> std::pair<polynomial<T>, polynomial<T>> {
  return {p / q, p % q};
}
/*
    description:
        Finds the factors of the leading coefficient of the polynomial.
    parameters:
        p - the polynomial
    return:
        std::vector<T> - a vector of factors of the leading coefficient
*/
template <typename T>
inline auto factor_last(polynomial<T> p) -> std::vector<T> {
  const size_t leading_quotient = p.coefficients[p.degree];
  std::vector<T> factor_of_last_term{};
  for (std::size_t i = 1; i <= leading_quotient; i++) {
    if (leading_quotient % i == 0) {
      factor_of_last_term.push_back(i);
    }
  }
  return factor_of_last_term;
}
/*
    description:
        Finds the factors of the constant term of the polynomial.
    parameters:
        p - the polynomial
    return:
        std::vector<T> - a vector of factors of the constant term
*/
template <typename T>
inline auto factor_first(polynomial<T> p) -> std::vector<T> {
  std::vector<T> factor_of_first_term{};
  for (std::size_t i = 1; i <= static_cast<std::size_t>(p.coefficients[0]);
       i++) {
    if (static_cast<int>(p.coefficients[0]) % static_cast<int>(i) == 0) {
      factor_of_first_term.push_back(i);
    }
  }
  return factor_of_first_term;
}
/*
    description:
        Finds the rational root candidates for the polynomial.
    parameters:
        p - the polynomial
    return:
        std::set<T> - a set of rational root candidates
*/
template <typename T>
inline auto root_rational_candidates(polynomial<T> p) -> std::set<T> {
  std::vector<T> factor_of_last_term = factor_last(p);
  std::vector<T> factor_of_first_term = factor_first(p);
  std::set<T> factors{};
  for (std::size_t i = 0; i < factor_of_first_term.size(); i++) {
    for (std::size_t j = 0; j < factor_of_last_term.size(); j++) {
      factors.emplace(factor_of_first_term[i] / factor_of_last_term[j]);
    }
  }
  return factors;
}
/*
    Description:
        Computes the greatest common divisor (GCD) of two polynomials using
        the Euclidean algorithm.

    Parameters:
        p - the first polynomial
        q - the second polynomial

    Return:
        std::pair<polynomial<T>, std::vector<T>> - a pair containing:
             - The GCD of the two polynomials.
             - A vector of degrees of the polynomials at each step of the
   algorithm.
*/
template <typename T>
inline auto gcd(polynomial<T> p, polynomial<T> q)
    -> std::pair<polynomial<T>, std::vector<T>> {
  std::vector<T> steps;
  while (q.degree > 0) {
    auto [quotient, remainder] = divide(p, q);
    p = q;
    q = remainder;
    steps.push_back(p.degree);
  }
  return {p, steps};
}
}  // namespace polynomial
/*
    Description:
        Demonstrates polynomial operations and prints the results.

    Functions:
        example_basic_operation:
            Performs and prints basic polynomial operations (+, -, *, /, %).

        example:
            Performs various operations on two polynomials, including division,
   evaluation, finding rational root candidates, and GCD. Prints the results.

        print_expected:
            Prints the expected results for the operations performed in example.

        test:
            Initializes example polynomials and a value for evaluation, calls
   example, and prints the expected results.

    Parameters:
        T - The type of the polynomial coefficients.

    Usage:
        Call test() to run the demonstrations and print results.
*/
template <typename T>
inline auto example_basic_operation(polynomial::polynomial<T> &p1,
                                    polynomial::polynomial<T> &p2) {
  std::print("Result + : {}\n", (p1 + p2).coefficients);
  std::print("Result - : {}\n", (p1 - p2).coefficients);
  std::print("Result * : {}\n", (p1 * p2).coefficients);
  std::print("Result / : {}\n", (p1 / p2).coefficients);
  std::print("Result % : {}\n", (p1 % p2).coefficients);
}

template <typename T>
inline auto example(polynomial::polynomial<T> &p1,
                    polynomial::polynomial<T> &p2, double value) {
  std::print("This function performs operations on two polynomials:\n");
  std::print("Polynomial p = {}, Polynomial q = {},\n", p1.coefficients,
             p2.coefficients);
  example_basic_operation(p1, p2);
  auto [quotient, remainder] = polynomial::divide(p1, p2);
  std::print("Division result (Quotient): {}\n", quotient.coefficients);
  std::print("Division result (Remainder): {}\n", remainder.coefficients);
  std::print("Evaluation at x = {}: {}\n", value, p1(value));
  std::print("Root rational candidates: {}\n",
             polynomial::root_rational_candidates(p1));
  std::print("GCD: {}\n", gcd(p1, p2).first.coefficients);
  std::print("Extended Euclidean Algorithm steps: {}\n\n\n",
             gcd(p1, p2).second);
}
/*
    Prints the expected results for the polynomial operations performed in the
   example function.
*/
void inline print_expected() {
  std::print("Expected + : [4, 5, 4]\n");
  std::print("Expected - : [2, 3, 4]\n");
  std::print("Expected *: [3, 7, 8, 4]\n");
  std::print("Expected / : [0, 4]\n");
  std::print("Expected % : [3, 0, 0]\n");
  std::print("Expected division (Quotient): [1, 4]\n");
  std::print("Expected division expected (Remainder): [2, 0, 0]\n");
  std::print("Expected evaluation at x = 2: 29\n");
  std::print("Expected root rational candidates: [0, 1, 3]\n");
  std::print("Expected GCD: [1]\n");
  std::print("Expected extended Euclidean Algorithm steps: [1, 0]\n");
}
/*
    description:
        Tests polynomial operations by initializing example polynomials and
   performing a series of operations on them, then prints both actual and
   expected results for verification.
*/
void inline test() {
  polynomial::polynomial<int> p1{{3, 4, 4}, 2};
  polynomial::polynomial<int> p2{{1, 1}, 1};
  const double value = 3.0;
  std::print("This function performs tests operations on two polynomials:\n");
  std::print("Polynomial p = {}, polynomial q = {},\n", p1.coefficients,
             p2.coefficients);
  example(p1, p2, value);
  print_expected();
}