#include <iostream>
#include "polynomial.hpp"

int main() {
	polynomial::polynomial<double> p1{{1, 2, 3}}; // x^2 + 2x + 3
    	polynomial::polynomial<double> p2{{1, 1}};    // x + 1
	
   	return 0;
}
