#include <cassert>
#include <iostream>
#include "Number_theory.hpp"

namespace Tests {
    bool test_gcd_function() {
        // Test 1: GCD function works correctly
        assert(number_theory::gcd(24, 36) == 12);
        assert(number_theory::gcd(7, 5) == 1);
        assert(number_theory::gcd(0, 5) == 5);
        assert(number_theory::gcd(18, 0) == 18);

        return true;
    }

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
        assert(number_theory::modular_inverse(3, 13) == 9);
        assert(number_theory::modular_inverse(5, 11) == 9);

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

        assert(number_theory::linear_congruence_solver(as, bs, mods) == 31);

        return true;
    }

    int test() {
        // Invoke all test functions and check if they return true
        assert(test_gcd_function());
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
