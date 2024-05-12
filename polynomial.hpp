#pragma once
#include <iostream>
#include <stdio.h>
#include <vector>
#include <utility>
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace polynomial {
    template <typename T>
    struct polynomial {
      public:
        std::vector<T> coefficients{};
        std::size_t degree{0};
      public:
        auto operator+=(const polynomial &p) -> polynomial & {
            if (p.degree > degree) { coefficients.resize(p.degree + 1, T{0}); }
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
        auto operator/=(const polynomial &p) -> polynomial & {
            if (p.degree == 0 && p.coefficients[0] == T{0}) {
                throw std::invalid_argument("Division by zero polynomial");
            }else if(p.degree > 1){
                throw std::invalid_argument("Division by invalid polynomial");
            }
            coefficients.resize(degree, T{0});
            degree -= 1;
            for(std::size_t i; i <= degree; i++){
                coefficients[i] /= p.coefficients[0];
                if( i+1 <= degree){
                    coefficients[i+1] -= coefficients[i] * p.coefficients[1];
                }
            }
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
    ->std::pair<polynomial<T>, polynomial<T>> {
    	polynomial<T> quotient, remainder;
        quotient = p / q;
        remainder = p % q;
        return std::make_pair(quotient, remainder);
    }
    template <typename T>
    inline auto root_rational_candidates(polynomial<T> p) -> std::vector<T>{
        size_t P = p.coefficients[p.degree];
        size_t Q = p.coefficients[0];
        std::vector<T> factors_of_P{};
        std::vector<T> factors_of_Q{};
        std::vector<T> factors{};
        for(std::size_t i = 1; i < P; i++){
            if(P % i == 0){
                factors_of_P.push_back(i);
            }
        }
        for(std::size_t i = 1; i < Q; i++){
            if(Q % i == 0){
                factors_of_Q.push_back(i);
            }
        }
        size_t duplicates = 0;
        for(std::size_t i = 0; i < factors_of_P.size; i++){
            for(std::size_t j = 0; j < factors_of_Q.size; j++){
                for(std::size_t k = 0; k < factors.size; k++){
                    if(factors_of_P[i] / factors_of_Q[j] == factors[k]){
                        duplicates = 1;
                        break;
                    }
                }
                if(duplicates == 0){
                    factors.push_back(factors_of_P[i] / factors_of_Q[j]);
                }
                duplicates = 0;
            }
        }
        size_t results_positive = 0;
        size_t results_negative = 0;
        for(std::size_t i = 0; i < factors.size; i++){
            for(std::size_t j = 0; j < p.degree; j++){
                results_positive += p.coefficients[j] * factors[i];
                results_negative += p.coefficients[j] * -factors[i];
            }
            if(results_positive == 0){
                return factors[i];
            }else if(results_negative == 0){
                return -factors[i];
            }
            results_positive = 0;
            results_negative = 0;
        }
        throw std::invalid_argument("Polynomial has no rational roots");
    }
    template <typename T>
    inline auto gcd(polynomial<T> p, polynomial<T> q) -> std::pair<polynomial<T>, std::vector<T>>
    {
	std::pair<polynomial<T>, std::vector<T>> result;
	polynomial<T> x_prev{1}, y_prev{0}, x{0}, y{1};
	std::vector<T> results;
    while (q.degree >= 0)
    {
        auto quotient = divide(p, q).first;
        p -= quotient * q;
        auto temp = std::move(x);
        x = x_prev - quotient * x;
        x_prev = std::move(temp);
        temp = std::move(y);
        y = y_prev - quotient * y;
        y_prev = std::move(temp);
        results.push_back(x_prev);
        results.push_back(y_prev);
    }
    result.first = x_prev; 
    result.second = std::move(results); 
    return result;
    }
}
void test() {
    polynomial::polynomial<double> p1{{1, 2, 3}};
    polynomial::polynomial<double> p2{{1, 1}};
    polynomial::polynomial<double> result_addition = p1 + p2;
    std::cout << "Addition result: ";
    for (auto coeff : result_addition.coefficients) {
        std::cout << coeff << " ";
    }
    std::cout << std::endl;
    polynomial::polynomial<double> result_subtraction = p1 - p2;
    std::cout << "Subtraction result: ";
    for (auto coeff : result_subtraction.coefficients) {
        std::cout << coeff << " ";
    }
    std::cout << std::endl;
    polynomial::polynomial<double> result_multiplication = p1 * p2;
    std::cout << "Multiplication result: ";
    for (auto coeff : result_multiplication.coefficients) {
        std::cout << coeff << " ";
    }
    std::cout << std::endl;
    polynomial::polynomial<double> result_division = p1 / p2;
    std::cout << "Division result: ";
    for (auto coeff : result_division.coefficients) {
        std::cout << coeff << " ";
    }
    std::cout << std::endl;
    polynomial::polynomial<double> result_modulus = p1 % p2;
    std::cout << "Modulus result: ";
    for (auto coeff : result_modulus.coefficients) {
        std::cout << coeff << " ";
    }
    std::cout << std::endl;
}
