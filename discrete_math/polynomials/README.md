## Introduction
This project implements a template-based C++ class for polynomial operations. The operations include addition, subtraction, multiplication, division, remainder, evaluation, root rational candidates, and finding the greatest common divisor (GCD) using the Euclidean algorithm. The code also includes a test suite to demonstrate the usage and verify the correctness of these operations.

## Polynomial Class
The polynomial class is a template class that supports operations on polynomials of any numerical type.

## Class Definition
code snippet that shows this:
namespace polynomial {
    template <typename T>
    struct polynomial {
    public:
        std::vector<T> coefficients{};
        std::size_t degree{};

    public:
        auto operator+=(const polynomial &p) -> polynomial &;
        auto operator-() const;
        auto operator-=(const polynomial &p) -> polynomial &;
        auto operator*=(const polynomial &p) -> polynomial &;
        auto operator/=(const polynomial &p) -> polynomial &;
        auto operator%=(const polynomial &p) -> polynomial &;
        auto operator()(T value);
    };
}

## Member Functions
operator +=: Adds another polynomial to the current polynomial.
operator -: Negates the polynomial.
operator -=: Subtracts another polynomial from the current polynomial.
operator *=: Multiplies the current polynomial by another polynomial.
operator /=: Divides the current polynomial by another polynomial.
operator %=: Computes the remainder when the current polynomial is divided by another polynomial.
operator (): Evaluates the polynomial at a given value.

## Non-member Functions
operator +: Adds two polynomials.
operator -: Subtracts the second polynomial from the first polynomial.
operator *: Multiplies two polynomials.
operator /: Divides the first polynomial by the second polynomial.
operator %: Computes the remainder of the first polynomial divided by the second polynomial.
divide: Divides the first polynomial by the second polynomial and returns both the quotient and the remainder.
factor_last: Finds the factors of the leading coefficient of the polynomial.
factor_first: Finds the factors of the constant term of the polynomial.
root_rational_candidates: Finds the rational root candidates for the polynomial.
gcd: Computes the greatest common divisor (GCD) of two polynomials using the Euclidean algorithm.

## Usage
The perform_operation function performs the specified operation on two polynomials, and the example function demonstrates the usage of polynomial operations.
code snippet that shows this:
void test() {
  polynomial::polynomial<int> p1{{3, 5, 4}}, p2{{1, 1}};
  p1.degree = 2;
  p2.degree = 1;

  std::print("This program performs tests operations on two polynomials:\n");
  std::print("Polynomial p = {}, polynomial q = {},\n", p1.coefficients, p2.coefficients);

  std::print("Result + : {}\n", perform_operation(p1, p2, '+').coefficients);
  std::print("Expected +: [4, 6, 4]\n");

  std::print("Result - : {}\n", perform_operation(p1, p2, '-').coefficients);
  std::print("Expected -: [2, 4, 4]\n");

  std::print("Result * : {}\n", perform_operation(p1, p2, '*').coefficients);
  std::print("Expected: [3, 8, 9, 4]\n");

  std::print("Result / : {}\n", perform_operation(p1, p2, '/').coefficients);
  std::print("Expected /: [1, 4]\n");

  std::print("Result % : {}\n", perform_operation(p1, p2, '%').coefficients);
  std::print("Expected %: [2, 0, 0]\n");

  auto [quotient, remainder] = polynomial::divide(p1, p2);

  std::print("Division result (Quotient): {}\n", quotient.coefficients );
  std::print("Division expected (Quotient): [1, 4]\n");

  std::print("Division result (Remainder): {}\n", remainder.coefficients);
  std::print("Division expected (Remainder): [2, 0, 0]\n");

  std::print("Evaluation at x = 2, result: {}\n", p1(2));
  std::print("Evaluation at x = 2, expected: 29\n");

  std::print("Root rational candidates: {}\n", polynomial::root_rational_candidates(p1) );
  std::print("Root rational candidates expected: [0, 1, 3]\n");

  auto gcd_result = polynomial::gcd(p1, p2);
  std::print("GCD: {}\n", gcd_result.first.coefficients);
  std::print("GCD expected: [2, 0, 0]\n");

  std::print("Extended Euclidean Algorithm steps: {}\n", gcd_result.second);
  std::print("Extended Euclidean Algorithm steps expected: [1, 0]\n");
}

## Conclusion
This project provides a robust implementation of polynomial operations in C++ using template programming. The provided functions cover a wide range of operations, making this implementation suitable for various mathematical and engineering applications. The accompanying test suite ensures the correctness of the implemented operations.

## Authors

- Jakub Kaźmierkiewicz
- Przemysław Boś
- Piotr Guzowski
- Alan Czerski