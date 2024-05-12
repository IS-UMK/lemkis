#pragma once
#include <iostream>
#include <type_traits>
#include <concepts>
#include <vector>
#include <cmath>
namespace number_theory
{
    // Function to calculate the greatest common divisor (GCD) of two integers.
    auto gcd(std::integral auto a, std::integral auto b) {
        while (b != 0) {
            int t = b;
            b = a % b;
            a = t;
        }
        return a;
    }

    // Function to check if a vector of numbers are pairwise coprime.
    bool are_coprime(std::vector<std::size_t> numbers)
    {
        for (std::size_t i = 0; i < numbers.size(); i++)
        {
            for (std::size_t j = i + 1; j < numbers.size(); j++)
            {
                if (gcd(numbers[i], numbers[j]) != 1)
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
        if (mod == 1) return 0;
        decltype(num) result = 1;
        num %= mod;

        while (exponent > 0)
        {
            if (exponent % 2 == 1)
                result = (result * num) % mod;
            num = (num * num) % mod;
            exponent /= 2;
        }
        return result;
    }

    /*finds (if exists) modular inverse of num,that is a number inv such that
     * num * inv = 1 modulo mod*/
    inline auto modular_inverse(std::integral auto num, std::size_t mod)
    {
        auto m = mod;

        if (mod == 0)
        {
            throw std::invalid_argument("The mod is not an integer.");
        }

        if (gcd(num, mod) != 1)
        {
            throw std::invalid_argument("The numbers must be coprime.");
        }


        auto a = num % m;
        auto m0 = m;
        auto t = 0, q = 0;
        auto x0 = 0, x1 = 1;

        if (m == 1)
            return 1;

        while (a > 1) {
            q = a / m;
            t = m;
            m = a % m;
            a = t;
            t = x0;
            x0 = x1 - q * x0;
            x1 = t;
            if (m == 0)
                break;
        }

        if (x1 < 0)
            x1 += m0;

        return x1;
    }

    /*find a solution x of ax = b modulo mod*/
    template <std::size_t mod>
    inline auto linear_congruence_solver(std::integral auto a, std::integral auto b)
    {
        if (mod == 0)
        {
            throw std::invalid_argument("mod cannot be zero.");
        }

        decltype(a) x;

        if (b % a == 0 && mod % a == 0)
        {
            x = (b / a) % (mod / a);
            return x;
        }

        if (gcd(a, mod) != 1)
        {
            throw std::invalid_argument("a and mod must be coprime.");
        }

        auto inverse = modular_inverse(a, mod);
        x = (b * inverse) % mod;
        return x;
    }

    /*returns primes smaller than up_to*/
    inline auto sieve_of_eratosthenes(std::size_t up_to)
    {
        std::vector<bool> is_prime(up_to + 1, true);
        std::vector<size_t> primes;

        for (size_t i = 2; i < up_to; i++)
        {
            if (is_prime[i])
            {
                for (size_t j = 2 * i; j < up_to; j += i)
                {
                    is_prime[j] = false;
                }
            }
        }

        for (size_t i = 2; i < up_to; i++)
        {
            if (is_prime[i])
            {
                primes.push_back(i);
            }
        }
        return primes;
    }

    /*decomposes a number into powers of primes*/
    inline auto decompose(std::integral auto value)
    {
        std::vector<std::pair<int, int>> primeFactors;

        for (int i = 2; i <= value / i; ++i)
        {
            int count = 0;
            while (value % i == 0)
            {
                ++count;
                value /= i;
            }
            if (count > 0)
            {
                primeFactors.emplace_back(i, count);
            }
        }
        if (value > 1)
        {
            primeFactors.emplace_back(value, 1);
        }
        return primeFactors;
    }

    /*calculates Euler’s Totient Function*/
    inline auto euler_totient(std::integral auto value)
    {
        decltype(value) result = 0;
        for (int i = 1; i <= value; i++) {
            if (gcd(i, value) == 1) {
                result++;
            }
        }
        return result;
    }

    /*finds the largest power pow of prime such that prime^pow divides value*/
    inline auto largest_power_of_prime_dividing_factorial(std::integral auto value, std::integral auto prime)
    {
        decltype(prime) pow = 0;
        while (value % prime == 0)
        {
            pow++;
            value /= prime;
        }
        return pow;
    }

    /*solves a system of linear equations as[i] * x = bs[i] mod mods[i].*/
    template <std::integral I>
    inline auto linear_congruence_solver(std::vector<I> as, std::vector<I> bs, std::vector<std::size_t> mods)
    {
        if (as.size() != bs.size() || bs.size() != mods.size())
        {
            throw std::invalid_argument("Input vectors must have the same size.");
        }


        for (size_t i = 0; i < as.size(); i++)
        {

            try
            {
                auto a_inverse = modular_inverse(as[i], mods[i]);
                bs[i] = (bs[i] * a_inverse) % mods[i];
            }
            catch (const std::invalid_argument& e)
            {
                auto a = as[i];
                auto mod = mods[i];
                auto b = bs[i];


                if (b % a == 0 && mod % a == 0)
                {
                    bs[i] = (b / a) % (mod / a);
                    mods[i] = (mod / a);
                }
                else
                {
                    throw std::invalid_argument("Mods must be coprime");
                }
            }

        }


        if (!are_coprime(mods))
        {
            throw std::invalid_argument("Mods must be coprime");
        }


        std::size_t m = 1;
        for (size_t i = 0; i < mods.size(); i++)
        {
            m *= mods[i];
        }

        auto result = 0;
        for (size_t i = 0; i < as.size(); i++)
        {
            size_t mi = m / mods[i];
            auto mi_inverse = modular_inverse(mi, mods[i]);
            result += (bs[i] * mi_inverse * mi);
        }

        return  result % m;
    }
}