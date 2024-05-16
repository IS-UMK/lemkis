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

namespace polynomial {
template <typename T>
struct polynomial {
 public:
  std::vector<T> coefficients{};
  std::size_t degree{};

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
  
  auto operator-() const {
    auto copy{*this};
    for (auto &c : copy.coefficients) {
      c *= -1;
    }
    return copy;
  }
  
  auto operator-=(const polynomial &p) -> polynomial & {
    return operator+=(-p);
  }
  
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
  
  auto operator%=(const polynomial &p) -> polynomial & {
    if (p.degree == 0 && p.coefficients[0] == T{0}) {
      throw std::invalid_argument("Division by zero polynomial");
    }
    polynomial quotient;
    polynomial remainder = *this;
    quotient = *this / p;
    remainder -= quotient * p;
    *this = remainder;
    return *this;
  }

 public:
  auto operator()(T value) {
    T result = 0;
    for (int i = this->degree; i >= 0; --i) {
      result = result * value + this->coefficients[i];
    }
    return result;
  }
};

template <typename T>
inline auto operator+(polynomial<T> p, polynomial<T> q) {
  p += q;
  return p;
}
template <typename T>
inline auto operator-(polynomial<T> p, polynomial<T> q) {
  p -= q;
  return p;
}
template <typename T>
inline auto operator*(polynomial<T> p, polynomial<T> q) {
  p *= q;
  return p;
}
template <typename T>
inline auto operator/(polynomial<T> p, polynomial<T> q) {
  p /= q;
  return p;
}
template <typename T>
inline auto operator%(polynomial<T> p, polynomial<T> q) {
  p %= q;
  return p;
}
template <typename T>
inline auto divide(polynomial<T> p, polynomial<T> q)
    -> std::pair<polynomial<T>, polynomial<T>> {
  return {p / q, p % q};
}

template <typename T>
inline auto root_rational_candidates(polynomial<T> p) -> std::set<T> {
  size_t leading_quotient = p.coefficients[p.degree];
  std::vector<T> factor_of_last_term{};
  std::vector<T> factor_of_first_term{};
  std::set<T> factors{};
  for (std::size_t i = 1; i <= leading_quotient; i++) {
    if (leading_quotient % i == 0) {
      factor_of_last_term.push_back(i);
    }
  }
  for (std::size_t i = 1; i <= static_cast<std::size_t>(p.coefficients[0]);
       i++) {
    if (static_cast<int>(p.coefficients[0]) % static_cast<int>(i) == 0) {
      factor_of_first_term.push_back(i);
    }
  }
  for (std::size_t i = 0; i < factor_of_first_term.size(); i++) {
    for (std::size_t j = 0; j < factor_of_last_term.size(); j++) {
      factors.emplace(factor_of_first_term[i] / factor_of_last_term[j]);
    }
  }
  return factors;
}

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

template <typename T>
std::ostream &operator<<(std::ostream &os, const polynomial<T> &p) {
  for (std::size_t i = p.degree; i > 0; --i) {
    os << p.coefficients[i] << "x^" << i << " + ";
  }
  os << p.coefficients[0];
  return os;
}
}

template <typename T>
void performoperation(const polynomial::polynomial<T> &p1,
                      const polynomial::polynomial<T> &p2,
                      const std::string &operation) {
  polynomial::polynomial<T> result;
  switch (operation[0]) {
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
  std::cout << operation << " result: ";
  for (auto coeff : result.coefficients) {
    std::cout << coeff << " ";
  }
  std::cout << std::endl;
}

void example() {
  polynomial::polynomial<double> p1{{3, 5, 4}}, p2{{1, 1}};
  p1.degree = 2;
  p2.degree = 1;
  std::cout << "This program performs operations on two polynomials" << std::endl;
  std::cout << "Polynomial p = " << p1 << ", polynomial q = " << p2
            << std::endl;
  performoperation(p1, p2, "+");
  performoperation(p1, p2, "-");
  performoperation(p1, p2, "*");
  performoperation(p1, p2, "/");
  performoperation(p1, p2, "%");
  auto [quotient, remainder] = polynomial::divide(p1, p2);
  auto print_coeffs = [](const auto &coeffs) {
    for (auto coeff : coeffs) std::cout << coeff << " ";
    std::cout << std::endl;
  };
  std::cout << "Division result (Quotient): ";
  print_coeffs(quotient.coefficients);
  std::cout << "Division result (Remainder): ";
  print_coeffs(remainder.coefficients);
  double value = 2.0;
  std::cout << "Evaluation at x = " << value << ": " << p1(value) << std::endl;
  std::cout << "Root rational candidates: ";
  for (auto root : polynomial::root_rational_candidates(p1))
    std::cout << root << " ";
  std::cout << std::endl
            << "GCD: " << polynomial::gcd(p1, p2).first << std::endl;
  std::cout << "Extended Euclidean Algorithm steps: ";
  for (auto &val : polynomial::gcd(p1, p2).second) std::cout << val << " ";
  std::cout << std::endl;
}
