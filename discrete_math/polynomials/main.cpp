#include <iostream>
#include "polynomial.hpp"

int main() {
    // Creating two polynomials
    polynomial::polynomial<int> p1{{3, 5, 4}, 2}; 
    polynomial::polynomial<int> p2{{1, 1}, 1}; 
    // x = value
    double value = 2.0;
    example(p1, p2, value);
    test();

    return 0;
}