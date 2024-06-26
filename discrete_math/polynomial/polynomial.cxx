module; // Indicates this is a module interface unit

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <format>
#include <iostream>
#include <numeric>
#include <set>
#include <stdexcept>
#include <utility>
#include <vector>

export module polynomial;

// The polynomial module provides a template-based polynomial class with various
// operator overloads to perform arithmetic operations, evaluation, and utility
// functions for working with polynomials.

// Polynomial structure: each element represent ax^k where a = p[i], k = i.
export template <typename T> class polynomial : public std::vector<T> {
private:
  using base_t = std::vector<T>;

public:
  // Constructor: Initialize a polynomial of a given degree
  // with all coefficients set to zero.
  explicit polynomial(std::size_t degree) : base_t(degree + 1, T{0}) {}

private:
  // Trim leading zeros from the polynomial.
  auto trim_polynomial(polynomial<T> &p) -> polynomial<T> & {
    while (p[p.degree()] == 0 && p.degree() != 0) {
      p.resize(p.degree(), T{0});
    }
    return p;
  }

  // Check if the polynomial is a zero polynomial.
  auto check_if_zero(const polynomial<T> &p) -> bool {
    for (std::size_t i = 0; i <= p.degree(); i++) {
      if (p[i] != 0) {
        return false;
      }
    }
    return true;
  }

public:
  // Get the degree of the polynomial.
  [[nodiscard]] auto degree() const -> std::size_t { return this->size() - 1; }

public:
  // Add a scalar value to the polynomial.
  auto operator+=(const T &val) -> polynomial<T> & {
    (*this)[0] += val;
    return trim_polynomial(*this);
  }

  // Add another polynomial to this polynomial.
  auto operator+=(const polynomial<T> &p) -> polynomial<T> & {
    if (p.degree() > degree()) {
      this->resize(p.degree() + 1, T{0});
    }
    for (std::size_t i = 0; i <= p.degree(); ++i) {
      (*this)[i] += p[i];
    }
    return trim_polynomial(*this);
  }

  // Unary minus operator to negate all coefficients.
  auto operator-() const {
    auto copy{*this};
    for (auto &c : copy) {
      c *= -1;
    }
    return copy;
  }

  // Subtract a scalar value from the polynomial.
  auto operator-=(const T &val) -> polynomial<T> & { return operator+=(-val); }

  // Subtract another polynomial from this polynomial.
  auto operator-=(const polynomial<T> &p) -> polynomial<T> & {
    return operator+=(-p);
  }

  // Multiply the polynomial by a scalar value.
  auto operator*=(const T &val) -> polynomial<T> & {
    for (std::size_t i = 0; i <= degree(); i++) {
      (*this)[i] *= val;
    }
    return trim_polynomial(*this);
  }

  // Multiply the polynomial by another polynomial.
  auto operator*=(const polynomial<T> &p) -> polynomial<T> & {
    polynomial<T> multiplication_polynomial{degree() + p.degree()};
    for (std::size_t i = 0; i <= degree(); i++) {
      for (std::size_t j = 0; j <= p.degree(); j++) {
        multiplication_polynomial[i + j] += (*this)[i] * p[j];
      }
    }
    *this = multiplication_polynomial;
    return trim_polynomial(*this);
  }

  // Divide the polynomial by a scalar value.
  auto operator/=(const T &val) -> polynomial<T> & {
    assert(val != 0);
    for (std::size_t i = 0; i <= degree(); i++) {
      (*this)[i] /= val;
    }
    return trim_polynomial(*this);
  }

  // Divide the polynomial by another polynomial.
  auto operator/=(const polynomial<T> &p) -> polynomial<T> & {
    assert(p.degree() <= degree() && !check_if_zero(p));
    polynomial<T> quo{degree() - p.degree()};
    for (std::size_t i = degree(); i >= p.degree(); i--) {
      quo[i - p.degree()] = (*this)[i] / p[p.degree()];
      for (std::size_t j = 0; j <= p.degree(); j++) {
        (*this)[i - j] -= quo[i - p.degree()] * p[p.degree() - 1];
      }
    }
    *this = quo;
    return trim_polynomial(*this);
  }

  // Modulus operation with a scalar value.
  auto operator%=(const T &val) -> polynomial<T> & {
    assert(val != 0);
    for (std::size_t i = 0; i <= degree(); i++) {
      (*this)[i] %= val;
    }
    return trim_polynomial(*this);
  }

  // Modulus operation with another polynomial.
  auto operator%=(const polynomial<T> &p) -> polynomial<T> & {
    *this -= (*this / p) * p;
    return trim_polynomial(*this);
  }

  // Evaluate the polynomial at a given value.
  auto operator()(T val) {
    T evaluation{0};
    for (std::size_t i = 0; i <= degree(); i++) {
      evaluation += (*this)[i] * pow(val, i);
    }
    return evaluation;
  }

public:
  // Friend functions to enable operations between
  // polynomials and scalars/polynomials.
  friend auto operator+(polynomial<T> p1, polynomial<T> p2) -> polynomial<T> {
    p1 += p2;
    return p1;
  }
  template <std::convertible_to<T> S>
  friend auto operator+(polynomial<T> p, const S &val) -> polynomial<T> {
    p += T{val};
    return p;
  }
  friend auto operator-(polynomial<T> p1, polynomial<T> p2) -> polynomial<T> {
    p1 -= p2;
    return p1;
  }
  template <std::convertible_to<T> S>
  friend auto operator-(polynomial<T> p, const S &val) -> polynomial<T> {
    p -= T{val};
    return p;
  }
  friend auto operator*(polynomial<T> p1, polynomial<T> p2) -> polynomial<T> {
    p1 *= p2;
    return p1;
  }
  template <std::convertible_to<T> S>
  friend auto operator*(polynomial<T> p, const S &val) -> polynomial<T> {
    p *= T{val};
    return p;
  }
  friend auto operator/(polynomial<T> p1, polynomial<T> p2) -> polynomial<T> {
    p1 /= p2;
    return p1;
  }
  template <std::convertible_to<T> S>
  friend auto operator/(polynomial<T> p, const S &val) -> polynomial<T> {
    p /= T{val};
    return p;
  }
  friend auto operator%(polynomial<T> p1, polynomial<T> p2) -> polynomial<T> {
    p1 %= p2;
    return p1;
  }
  template <std::convertible_to<T> S>
  friend auto operator%(polynomial<T> p, const S &val) -> polynomial<T> {
    p %= T{val};
    return p;
  }
};

// Namespace for utility functions related to polynomials.
export namespace utils::polynomial {

// Divide one polynomial by another and return the quotient and remainder.
template <typename T>
inline auto divide(const ::polynomial<T> &p1, const ::polynomial<T> &p2)
    -> std::pair<::polynomial<T>, ::polynomial<T>> {
  return {p1 / p2, p1 % p2};
}

// Find the factors of a specific term in a polynomial.
template <typename T>
inline auto factors_of_term(::polynomial<T> &p,
                            std::size_t term) -> std::vector<T> {
  assert(0 <= term && term <= p.degree());
  std::vector<T> factors;
  for (std::size_t i = 1; i <= p[term]; i++) {
    if (static_cast<int>(p[term]) % static_cast<int>(i) == 0) {
      factors.push_back(i);
    }
  }
  return factors;
}

// Get the rational root candidates for a polynomial.
template <typename T>
inline auto root_rational_candidates(::polynomial<T> p) -> std::set<T> {
  std::vector<T> factors_of_last{factors_of_term(p, 0)};
  std::vector<T> factors_of_first{factors_of_term(p, p.degree())};
  std::set<T> candidates{};
  for (std::size_t i = 0; i < factors_of_last.size(); i++) {
    for (std::size_t j = 0; j < factors_of_first.size(); j++) {
      candidates.emplace(factors_of_last[i] / factors_of_first[j]);
    }
  }
  return candidates;
}

// Compute the greatest common divisor (GCD) of two polynomials and record the
// steps.

template <typename T>
inline auto gcd(::polynomial<T> p1, ::polynomial<T> p2)
    -> std::pair<::polynomial<T>, std::vector<T>> {
  std::vector<T> steps;
  while (p2.degree() > 0) {
    auto [quotient, remainder] = divide(p1, p2);
    p1 = p2;
    p2 = remainder;
    steps.push_back(p1.degree());
  }
  return {p1, steps};
}
} // namespace utils::polynomial

// Format a polynomial term for output.
inline auto format_term(auto entry /*p[i]*/, auto degree) -> std::string {
  if (entry == 0) {
    return "";
  }
  return (abs(entry) == 1) ? std::format("x^{}", degree)
                           : std::format("{}x^{}", abs(entry), degree);
}

// Determine the sign of the next term for output.
inline auto sign_next(auto entry /*p[i-1]*/) -> std::string {
  if (entry == 0) {
    return "";
  }
  return (entry < 0) ? " - " : " + ";
}

// Convert a polynomial to a string representation.
template <typename T> auto to_string(polynomial<T> p) {
  std::string out{};
  if (p.degree() > 0) {
    for (std::size_t i = p.degree(); i > 1; i--) {
      out += format_term(p[i], i);
      out += sign_next(p[i - 1]);
    }
    if (p[1] != 0) {
      out += (abs(p[1]) == 1) ? "x" : std::format("{}x", abs(p[1]));
    }
    out += sign_next(p[0]);
  }
  out += (p[0] != 0) ? std::format("{}", abs(p[0])) : "";
  return out;
}

// Custom formatter for polynomial class to enable std::format.
template <typename T> struct std::formatter<polynomial<T>, char> {

  template <class ParseContext>
  constexpr auto parse(ParseContext &ctx) -> ParseContext::iterator {
    auto pos = ctx.begin();
    if (pos == ctx.end()) {
      return pos;
    }
    if (*pos != '}') {
      ++pos;
    }
    return pos;
  }

  template <class FmtContext>
  auto format(polynomial<T> p, FmtContext &ctx) const -> FmtContext::iterator {
    return std::format_to(ctx.out(), "{}", ::to_string(p));
  }
};
