#pragma once
#include <stdio.h>

#include <algorithm>
#include <cmath>
#include <iostream>
#include <print>
#include <set>
#include <stdexcept>
#include <utility>
#include <vector>
#include <numeric>
#include <format>

namespace polynomial {
template <typename T> struct polynomial {
public:
  std::vector<T> coefficients{};
  std::size_t degree{};
  /*
      description:
          Adds another polynomial to the current polynomial.
      members:
          const polynomial &p - polynomial to be added to the current
     polynomial. methods: polynomial& operator+=(const polynomial &p) - performs
     the addition and updates the current polynomial.
  */
public:
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
      members:
          None
      methods:
          polynomial operator-() const - returns a new polynomial which is the
     negation of the current polynomial.
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
          Subtracts another polynomial from the current polynomial.
      members:
          const polynomial &p - polynomial to be subtracted from the current
     polynomial. methods: polynomial& operator-=(const polynomial &p) - performs
     the subtraction and updates the current polynomial.
  */
  auto operator-=(const polynomial &p) -> polynomial & {
    return operator+=(-p);
  }
  /*
      description:
          Multiplies the polynomial by another polynomial.
      members:
          const polynomial &p - polynomial to multiply with the current
     polynomial. methods: polynomial& operator*=(const polynomial &p) - performs
     the multiplication and updates the current polynomial.
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
          Divides the polynomial by another polynomial.
      members:
          const polynomial &p - polynomial to divide the current polynomial by.
      methods:
          polynomial& operator/=(const polynomial &p) - performs the division
     and updates the current polynomial. throws: std::invalid_argument - if the
     input polynomial is zero or has a higher degree than the current
     polynomial.
  */
  auto operator/=(const polynomial &p) -> polynomial & {
    if (p.degree == 0 && p.coefficients[0] == T{0}) {
      throw std::invalid_argument("Division by zero polynomial");
    } else if (p.degree > degree) {
      throw std::invalid_argument("Division by invalid polynomial");
    }
    std::vector<T> division_vector{};
    division_vector.resize(degree - p.degree + 1, T{0});
    for (std::size_t i = degree; i > p.degree - 1; i--) {
      division_vector[i - p.degree] =
          coefficients[i] / p.coefficients[p.degree];
      if (i - 1 > 0) {
        for (std::size_t j = p.degree + 1; j > 0; j--) {
          coefficients[j + i - p.degree - 1] -=
              division_vector[i - p.degree] * p.coefficients[j - 1];
        }
      }
    }
    coefficients.resize(degree - p.degree + 1, T{0});
    degree -= p.degree;
    coefficients = division_vector;
    return *this;
  }
  /*
      description:
          Computes the remainder of the polynomial division by another polynomial. 
     members: 
      const polynomial &p - polynomial to divide the current polynomial by. 
     methods: 
     polynomial& operator%=(const polynomial &p) -
     performs the modulo operation and updates the current polynomial. throws:
          std::invalid_argument - if the input polynomial is zero.
  */
  auto operator%=(const polynomial &p) -> polynomial & {
    if (p.degree == 0 && p.coefficients[0] == T{0}) {
      throw std::invalid_argument("Division by zero polynomial");
    }
    *this -= (*this/p)*p;
    return *this;
  }
  /*
      description:
          Evaluates the polynomial at a given value.
      members:
          T value - the value at which to evaluate the polynomial.
      methods:
          T operator()(T value) - returns the result of the polynomial
     evaluation.
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
        Adds two polynomials and returns the result.
    members:
        polynomial<T> p - the first polynomial.
        polynomial<T> q - the second polynomial.
    methods:
        polynomial<T> operator+(polynomial<T> p, polynomial<T> q) - returns a
   new polynomial which is the sum of p and q.
*/
template <typename T> inline auto operator+(polynomial<T> p, polynomial<T> q) {
  p += q;
  return p;
}
/*
    description:
        Subtracts one polynomial from another and returns the result.
    members:
        polynomial<T> p - the first polynomial.
        polynomial<T> q - the polynomial to be subtracted from the first
   polynomial. methods: polynomial<T> operator-(polynomial<T> p, polynomial<T>
   q) - returns a new polynomial which is the difference of p and q.
*/
template <typename T> inline auto operator-(polynomial<T> p, polynomial<T> q) {
  p -= q;
  return p;
}
/*
    description:
        Multiplies two polynomials and returns the result.
    members:
        polynomial<T> p - the first polynomial.
        polynomial<T> q - the second polynomial.
    methods:
        polynomial<T> operator*(polynomial<T> p, polynomial<T> q) - returns a
   new polynomial which is the product of p and q.
*/
template <typename T> inline auto operator*(polynomial<T> p, polynomial<T> q) {
  p *= q;
  return p;
}
/*
    description:
        Divides one polynomial by another and returns the result.
    members:
        polynomial<T> p - the first polynomial (dividend).
        polynomial<T> q - the second polynomial (divisor).
    methods:
        polynomial<T> operator/(polynomial<T> p, polynomial<T> q) - returns a
   new polynomial which is the quotient of p divided by q.
*/
template <typename T> inline auto operator/(polynomial<T> p, polynomial<T> q) {
  p /= q;
  return p;
}
/*
    description:
        Computes the remainder of one polynomial divided by another and returns
   the result. members: polynomial<T> p - the first polynomial (dividend).
        polynomial<T> q - the second polynomial (divisor).
    methods:
        polynomial<T> operator%(polynomial<T> p, polynomial<T> q) - returns a
   new polynomial which is the remainder of p divided by q.
*/
template <typename T> inline auto operator%(polynomial<T> p, polynomial<T> q) {
  p %= q;
  return p;
}
/*
    description:
        Divides one polynomial by another and returns both the quotient and the
   remainder. members: polynomial<T> p - the first polynomial (dividend).
        polynomial<T> q - the second polynomial (divisor).
    methods:
        std::pair<polynomial<T>, polynomial<T>> divide(polynomial<T> p,
   polynomial<T> q) - returns a pair of polynomials, the first is the quotient
   and the second is the remainder of p divided by q.
*/
template <typename T>
inline auto divide(polynomial<T> p, polynomial<T> q)
    -> std::pair<polynomial<T>, polynomial<T>> {
  return {p / q, p % q};
}
/*
    description:
        Finds the rational root candidates of a polynomial using the Rational
   Root Theorem. members: polynomial<T> p - the polynomial for which to find the
   rational root candidates. methods: std::set<T>
   root_rational_candidates(polynomial<T> p) - returns a set of rational numbers
   that are possible roots of the polynomial.
*/
template <typename T>
inline auto factor_last(polynomial<T> p) -> std::vector<T> {
  size_t leading_quotient = p.coefficients[p.degree];
  std::vector<T> factor_of_last_term{};
  for (std::size_t i = 1; i <= leading_quotient; i++) {
    if (leading_quotient % i == 0) {
      factor_of_last_term.push_back(i);
    }
  }
  return factor_of_last_term;
}

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
    description:
        Computes the greatest common divisor (GCD) of two polynomials using the
   Euclidean algorithm. members: polynomial<T> p - the first polynomial.
        polynomial<T> q - the second polynomial.
    methods:
        std::pair<polynomial<T>, std::vector<T>> gcd(polynomial<T> p,
   polynomial<T> q) - returns a pair containing the GCD of the two polynomials
   and a vector of the degrees of the remainders at each step of the Euclidean
   algorithm.
*/
template <typename T>
auto gcd(polynomial<T> p, polynomial<T> q)
    -> std::pair<polynomial<T>, std::vector<T>> {
  std::vector<T> steps;
  while (q.degree < p.degree) {
    auto [quotient, remainder] = divide(p, q);
    p = q;
    q = remainder;
    steps.push_back(p.degree);
  }
  return {p, steps};
}
/*
    description:
        Outputs the polynomial to an output stream in human-readable format.
    members:
        std::ostream &os - the output stream.
        const polynomial<T> &p - the polynomial to output.
    methods:
        std::ostream& operator<<(std::ostream &os, const polynomial<T> &p) -
   writes the polynomial to the output stream and returns the stream.
*/
template <typename T>
std::ostream &operator<<(std::ostream &os, const polynomial<T> &p) {
  for (std::size_t i = p.degree; i > 0; --i) {
    os << p.coefficients[i] << "x^" << i << " + ";
  }
  os << p.coefficients[0];
  return os;
}
}
/*
    description:
        Performs a specified operation on two polynomials and prints the result.
    members:
        const polynomial::polynomial<T> &p1 - the first polynomial.
        const polynomial::polynomial<T> &p2 - the second polynomial.
        const std::string &operation - the operation to perform, which can be
   "+", "-", "*", "/", or "%". methods: void perform_operation - performs the
   specified operation on p1 and p2 and prints the result.
*/
template <typename T>
void perform_operation(const polynomial::polynomial<T> &p1,
                      const polynomial::polynomial<T> &p2,
                      const char &operation) {
  polynomial::polynomial<T> result;
  switch (operation) {
  case '+':
    result = p1 + p2;
    break;
  case '-':
    result = p1 - p2;
    break;
  case '*':
    result = p1 * p2;
    break;
  case '/':
    result = p1 / p2;
    break;
  case '%':
    result = p1 % p2;
    break;
  default:
    std::cerr << "Unsupported operation: " << operation << std::endl;
    return;
  }
  std::print("result {} : {}\n", operation, result.coefficients);
}

/*
    description:
        Prints the coefficients of a polynomial.
    members:
        const std::vector<T> &coeffs - the coefficients of the polynomial.
    methods:
        void print_coeffs(const std::vector<T> &coeffs) - prints the
        coefficients.
*/
template <typename T> void print_coeffs(const std::vector<T> &coeffs) {
  for (const auto &coeff : coeffs) {
    std::print("{} ", coeff);
  }
  std::print("\n");
}

void example( const polynomial::polynomial<T> &p1, const polynomial::polynomial<T> &p2) {
  polynomial::polynomial<double> p1{{3, 5, 4},{2}}, p2{{1, 1}};
  p1.degree = 2;
  p2.degree = 1;

  std::print("This program performs operations on two polynomials\n");
  std::print("Polynomial p = {}, polynomial q = {}\n", p1.coefficients, p2.coefficients);

  for (const auto &op : {'+', '-', '*', '/', '%'}) {
    perform_operation(p1, p2, op);
  }

  auto [quotient, remainder] = polynomial::divide(p1, p2);

  std::print("Division result (Quotient): {}\n", quotient.coefficients );

  std::print("Division result (Remainder): {}\n", remainder.coefficients);

  double value = 2.0;
  std::print("Evaluation at x = {}: {}\n", value, p1(value));

  std::print("Root rational candidates: {}\n", polynomial::root_rational_candidates(p1) );
  
  auto gcd_result = polynomial::gcd(p1, p2);
  std::print("GCD: {}\n", gcd_result.first.coefficients);


  std::print("Extended Euclidean Algorithm steps: {}\n", gcd_result.second);
}

void test() {
  polynomial::polynomial<double> p1{{3, 5, 4}}, p2{{1, 1}};
  p1.degree = 2;
  p2.degree = 1;

  std::print("This program performs tests operations on two polynomials\n");
  std::print("Polynomial p = {}, polynomial q = {}\n", p1.coefficients, p2.coefficients);

  perform_operation(p1, p2, '+');
  std::print("Expected +: [4, 6, 4]\n");

  perform_operation(p1, p2, '-');
  std::print("Expected -: [2, 4, 4]\n");

  perform_operation(p1, p2, '*');
  std::print("Expected *: [3, 8, 9, 4]\n");

  perform_operation(p1, p2, '/');
  std::print("Expected /: [1, 4]\n");

  perform_operation(p1, p2, '%');
  std::print("Expected %: [2, 0, 0]\n");

  auto [quotient, remainder] = polynomial::divide(p1, p2);

  std::print("Division result (Quotient): {}\n", quotient.coefficients );
  std::print("Division expected (Quotient): [1, 4]\n");

  std::print("Division result (Remainder): {}\n", remainder.coefficients);
  std::print("Division expected (Remainder): [2, 0, 0]\n");

  double value = 2.0;
  std::print("Evaluation at x = {} result: {}\n", value, p1(value));
  std::print("Evaluation at x = {} expected: 29\n", value);

  std::print("Root rational candidates: {}\n", polynomial::root_rational_candidates(p1) );
  std::print("Root rational candidates expected: [0, 1, 3]\n");

  auto gcd_result = polynomial::gcd(p1, p2);
  std::print("GCD: {}\n", gcd_result.first.coefficients);
  std::print("GCD expected: [2, 0, 0]\n");

  std::print("Extended Euclidean Algorithm steps: {}\n", gcd_result.second);
  std::print("Extended Euclidean Algorithm steps expected: [1, 0]\n");
}