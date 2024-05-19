#pragma once
#include <iostream>
#include <type_traits>
#include <concepts>
#include <vector>
#include <cmath>
#include <cassert>
#include <numeric>
#include "euclidean.hpp"
namespace number_theory
{
    // Function to check if a vector of numbers are pairwise coprime.
    bool are_coprime(std::vector<std::size_t> numbers){
        for (std::size_t i = 0; i < numbers.size(); i++){
            for (std::size_t j = i + 1; j < numbers.size(); j++){
                if (std::gcd(numbers[i], numbers[j]) != 1)
                {
                    return false;
                }
            }
        }
        return true;
    }
    /*finds num ^ exponent modulo mod, ensure that it is of O(log(exponent))
     * complexity*/
    template <std::size_t mod>
    inline auto modular_pow(std::integral auto num, std::size_t exponent)
    {
        using T = std::remove_cvref_t<decltype(num)>;
        if (mod == 1) return 0;
        T result{ 1 };
        num %= mod;
        while (exponent > 0) {
            if (exponent % 2 == 1)
                result = (result * num) % mod;
            num = (num * num) % mod;
            exponent /= 2;
        }
        return result;
    }
    /*finds (if exists) modular inverse of num,that is a number inv such that
     * num * inv = 1 modulo mod*/
     // extended_gcd?
    template <std::size_t mod>
    inline auto modular_inverse(std::integral auto num) {
        auto [result, _] = algorithms::gcd_extended(num, mod);
        auto [coefficients, gcd] = result;
        auto [x, _unused] = coefficients;
        int modulo = mod;
        if (gcd != 1) {
            throw std::invalid_argument("Inverse does not exist.");
        }
        return ((x % modulo) + modulo) % modulo;
    }
    /*find a solution x of ax = b modulo mod*/
    template <std::size_t mod>
    inline auto linear_congruence_solver(std::integral auto a, std::integral auto b) {
        if (b % a == 0 && mod % a == 0) {
            return (b / a) % (mod / a);
        }
        return (b * number_theory::modular_inverse<mod>(a) % mod);
    }
    /*returns primes smaller than up_to*/
    inline auto sieve_of_eratosthenes(std::size_t up_to) {
        std::vector<bool> is_prime(up_to + 1, true);
        std::vector<size_t> primes;
        for (size_t i = 2; i < up_to; i++) {
            if (is_prime[i]) {
                for (size_t j = 2 * i; j < up_to; j += i) {
                    is_prime[j] = false;
                }
            }
        }
        for (size_t i = 2; i < up_to; i++) {
            if (is_prime[i]) {
                primes.push_back(i);
            }
        }
        return primes;
    }
    /*decomposes a number into powers of primes*/
    inline auto decompose(std::integral auto value) {
        std::vector<std::pair<int, int>> primeFactors;
        for (int i = 2; i <= value / i; ++i) {
            int count = 0;
            while (value % i == 0) {
                ++count;
                value /= i;
            }
            if (count > 0) {
                primeFactors.emplace_back(i, count);
            }
        }
        if (value > 1) {
            primeFactors.emplace_back(value, 1);
        }
        return primeFactors;
    }
    /*calculates Euler’s Totient Function*/
    inline auto euler_totient(std::integral auto value) {
    std:size_t result = 0;
        for (int i = 1; i <= value; i++) {
            if (std::gcd(i, value) == 1) {
                result++;
            }
        }
        return result;
    }
    /*finds the largest power pow of prime such that prime^pow divides value*/
    inline auto largest_power_of_prime_dividing_factorial(std::integral auto value, std::integral auto prime) {
        using I = decltype(prime);
        I pow{ 0 };
        while (value % prime == 0) {
            pow++;
            value /= prime;
        }
        return pow;
    }
    // Function to solve a system of linear congruences
    template <std::integral I>
    inline auto linear_congruence_solver(std::vector<I> as, std::vector<I> bs, std::vector<std::size_t> mods) {
        assert(as.size() == bs.size() && bs.size() == mods.size());
        using T = std::common_type_t<I, std::size_t>;
        T x = 0;
        T M = 1;
        for (const auto& mod : mods) {
            M *= static_cast<T>(mod);
        }
        for (std::size_t i = 0; i < as.size(); ++i) {
            T a = static_cast<T>(as[i]);
            T b = static_cast<T>(bs[i]);
            T mod = static_cast<T>(mods[i]);

            T Mi = M / mod;
            T Mi_inv = static_cast<T>(number_theory::modular_inverse<static_cast<std::size_t>(mod)>(Mi));
            x = (x + b * Mi * Mi_inv) % M;
        }
        return (x % M + M) % M;
    }
}
namespace tests {
    bool test_coprime_function() {
        // Test 2: Coprime function correctly identifies coprime numbers
        std::vector<std::size_t> coprime_numbers = { 15, 28, 37 };
        std::vector<std::size_t> non_coprime_numbers = { 15, 28, 36 };
        assert(number_theory::are_coprime(coprime_numbers) == true);
        assert(number_theory::are_coprime(non_coprime_numbers) == false);
        return true;
    }
    bool test_modular_pow_function() {
        // Test 3: Modular pow function returns correct results
        assert(number_theory::modular_pow<10>(2, 3) == 8);
        assert(number_theory::modular_pow<7>(4, 3) == 1);
        return true;
    }
    bool test_modular_inverse_function() {
        // Test 4: Modular inverse function works correctly
        //std::cout << "Wynik modular_inverse: " << number_theory::modular_inverse<13>(3) << std::endl;
        assert(number_theory::modular_inverse<13>(3) == 9);
        assert(number_theory::modular_inverse<11>(5) == 9);
        return true;
    }
    bool test_linear_congruence_solver_function() {
        // Test 5: Linear congruence solver returns correct results
        assert(number_theory::linear_congruence_solver<13>(3, 5) == 6);
        assert(number_theory::linear_congruence_solver<7>(2, 5) == 6);
        return true;
    }
    bool test_sieve_of_eratosthenes_function() {
        // Test 6: Sieve of Eratosthenes generates prime numbers correctly
        auto primes = number_theory::sieve_of_eratosthenes(50);
        std::vector<size_t> expected_primes = { 2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47 };
        assert(primes == expected_primes);
        return true;
    }
    bool test_prime_factor_decomposition_function() {
        // Test 7: Prime factor decomposition returns correct results
        auto factors = number_theory::decompose(90);
        std::vector<std::pair<int, int>> expected_factors = { {2, 1}, {3, 2}, {5, 1} };
        assert(factors == expected_factors);
        return true;
    }
    bool test_euler_totient_function() {
        // Test 8: Euler totient function calculates phi correctly
        assert(number_theory::euler_totient(10) == 4);
        assert(number_theory::euler_totient(15) == 8);
        return true;
    }
    bool test_largest_power_of_prime_dividing_factorial_function() {
        // Test 9: Largest power of prime dividing factorial is calculated correctly
        assert(number_theory::largest_power_of_prime_dividing_factorial(10, 2) == 1);
        assert(number_theory::largest_power_of_prime_dividing_factorial(10, 3) == 0);
        return true;
    }
    bool test_linear_congruence_solver_multiple_function() {
        // Test 10: Linear congruence solver for multiple equations returns correct results
        std::vector<int> as = { 3, 4 };
        std::vector<int> bs = { 2, 3 };
        std::vector<size_t> mods = { 7, 11 };
        std::cout << "wynik!: " << number_theory::linear_congruence_solver(as, bs, mods) << std::endl;
        assert(number_theory::linear_congruence_solver(as, bs, mods) == 31);
        return true;
    }
    int test() {
        // Invoke all test functions and check if they return true
        assert(test_coprime_function());
        assert(test_modular_pow_function());
        assert(test_modular_inverse_function());
        assert(test_linear_congruence_solver_function());
        assert(test_sieve_of_eratosthenes_function());
        assert(test_prime_factor_decomposition_function());
        assert(test_euler_totient_function());
        assert(test_largest_power_of_prime_dividing_factorial_function());
        assert(test_linear_congruence_solver_multiple_function());
        // Print success message if all tests pass
        std::cout << "All tests passed successfully!" << std::endl;
        return 0;
    }
}