#pragma once
#include <iostream>
#include <stdio.h>
#include <vector>
#include <utility>
#include <algorithm>

namespace polynomial {

    /* x + 1 + (-x) = 1*/
    // a_n x^n + a_{n - 1}x^{n-1} + ... + a_1 x_1 + a_0
    // degree = n o ile a_n != 0
    // ale mamy n + 1 współczynników
    /*post and pre increment operator*/
    template <typename T>
    struct polynomial {
      public:
        std::vector<T> coefficients{};
        std::size_t degree{0};

      public:
        /*adds, subtracks, multiplies this by polynomial p*/
        auto operator+=(const polynomial &p) -> polynomial & {
            if (p.degree > degree) { coefficients.resize(p.degree + 1, T{0}); }
            //     degree = p.degree;
            //     for (std::size_t i = 0; i <= p.degree; ++i) {
            //         coefficients[i] += p.coefficients[i];
            //     }
            // } else /*p.degree <= degree)*/ {
            //     for (std::size_t i = 0; i <= p.degree; ++i) {
            //         coefficients[i] += p.coefficients[i];
            //         if (coefficients[i] != 0) { degree = i; }
            //     }
            // }
            for (std::size_t i = 0; i <= p.degree; ++i) {
                coefficients[i] += p.coefficients[i];
                if (coefficients[i] != 0) { degree = i; }
            }
            return *this;
        }

        auto operator-() const {
            auto copy{*this};
            for (auto &c : copy.coefficients) { c *= -1; }
            return copy;
        }
        

        auto operator-=(const polynomial &p) -> polynomial & {
            return operator+=(-p);
        }

        auto operator*=(const polynomial &p) -> polynomial &{
            coefficients.resize(p.degree + degree + 1, T{0});
            for (std::size_t i = 0; i < degree; ++i)
            {
                for (std::size_t j = 0; j < p.degree; ++j)
                {
                    coefficients[i+j] += coefficients[i] * p.coefficients[j];
                }
                
            }
            degree += p.degree;
            return *this;
            
        }
        /*returns the quotient*/
        auto operator/=(const polynomial &p) -> polynomial & {
            if (p.degree == 0 && p.coefficients[0] == T{0}) {
                throw std::invalid_argument("Division by zero polynomial");
            }

            polynomial quotient;
            polynomial remainder = *this;

            while (remainder.degree >= p.degree) {
                T leading_coefficient = remainder.coefficients[remainder.degree] / p.coefficients[p.degree];
                size_t current_degree = remainder.degree - p.degree;

                std::vector<T> temp(current_degree + 1, T{0});
                temp[current_degree] = leading_coefficient;

                polynomial temp_polynomial;
                temp_polynomial.coefficients = temp;
                temp_polynomial.degree = current_degree;

                quotient += temp_polynomial;
                remainder -= temp_polynomial * p;
            }

            *this = quotient;
            return *this;
        }

        /*return the remainder*/
        auto operator%=(const polynomial &p) -> polynomial & {
            if (p.degree == 0 && p.coefficients[0] == T{0}) {
                throw std::invalid_argument("Division by zero polynomial");
            }

            // polynomial division
            polynomial quotient;
            polynomial remainder = *this;
            quotient = *this / p;

            // polynomial remainder
            remainder -= quotient * p;

            *this = remainder;
            return *this;
        }


      public:
        // evaluates this polynomial on the given value - Horner's algorithm
        auto operator()(T value);


    };

    /*analogs of member methods of polymial class*/
    template <typename T>
    inline auto operator+(polynomial<T> p, polynomial<T> q) {
        p += q;
        return p;
    }


    template <typename T>
    inline auto operator-(polynomial<T> p, polynomial<T> q){
           p -= q;
        return p;
    }

    template <typename T>
    inline auto operator*(polynomial<T> p, polynomial<T> q);


    template <typename T>
    inline auto operator/(polynomial<T> p, polynomial<T> q);


    template <typename T>
    inline auto operator%(polynomial<T> p, polynomial<T> q);
    /*returns quotient and remainder when dividing *this by p*/


    template <typename T>
    inline auto divide(polynomial<T> p, polynomial<T> q)
    ->std::pair<polynomial<T>, polynomial<T>> { return {p / q, p % q}; }

    template <typename T>
    struct fraction {
        T numerator{0};
        T denominator{1};
    };


    /*returns rational root candidades for polynomial<T> p. It is up to you
    how you define fraction (see return type std::vector<fraction>). You
    can/should use your classmates struct from representation project.*/
    template <typename T>
    inline auto root_rational_candidates(polynomial<T> p)
        -> std::vector<T>;


    template <typename T>
    inline auto gcd(polynomial<T> p, polynomial<T> q);
}  // namespace polynomial
