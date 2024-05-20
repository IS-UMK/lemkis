
# Number Theory Library

This is a C++ library providing various functions and algorithms related to number theory.

## Features

- **Pairwise Coprime Check:** Function to check if a vector of numbers are pairwise coprime.
- **Modular Exponentiation:** Compute `num^exponent % mod` efficiently.
- **Modular Inverse:** Find the modular inverse of a number.
- **Linear Congruence Solver:** Solve linear congruences.
- **Sieve of Eratosthenes:** Generate prime numbers efficiently using the Sieve of Eratosthenes algorithm.
- **Prime Factor Decomposition:** Decompose a number into its prime factors.
- **Euler's Totient Function:** Calculate Euler’s Totient Function.
- **Largest Power of Prime Dividing Factorial:** Find the largest power of a prime that divides a factorial.
- **Linear Congruence Solver for Multiple Equations:** Solve a system of linear congruences.

## Usage

To use this library, include the necessary header files and link with the compiled library. For example:

```cpp
#include "number_theory.hpp"
#include <iostream>

int main() {
    // Example usage
    std::vector<size_t> primes = number_theory::sieve_of_eratosthenes(100);
    for (auto prime : primes) {
        std::cout << prime << " ";
    }
    std::cout << std::endl;

    return 0;
}
```

## Compilation

### Using g++
To compile your program using `g++`, you can use the following command:
```bash
g++ -std=c++17 -o main main.cpp
```
Replace `main.cpp` with the name of your source file.
### Using clang
To compile your program using `clang`, you can use the following command:
```bash
clang++ -std=c++17 -o main main.cpp
```
Replace `main.cpp` with the name of your source file.
## Testing
The library includes a set of test cases to ensure the correctness of its functions. To run the tests, compile and execute the test() function in the tests namespace.
```cpp
#include "number_theory.hpp"
#include "tests.hpp"

int main() {
    // Run tests
    tests::test();

    return 0;
}

```

## License
This project is licensed under the MIT License 

