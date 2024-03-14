#include "representation.h"

using namespace representation;

int main(){
	int numerator, denominator, base;
	Rep mc;
	Fraction fraction;
	Expansion expansion;

	numerator = mc.load_numerator();
	denominator = mc.load_denominator();
	base = mc.load_base();

	fraction = mc.create_fraction(numerator, denominator);

	std::cout << "\nLicznik:\t" << fraction.numerator;
	std::cout << "\nMianownik\t" << fraction.denominator;

	expansion = mc.create_expansion(base);

	std::cout << "\nBaza:\t" << expansion.base;

	std::cout << "\nWynik zamiany:\t" << mc.expansion_in_base(fraction, expansion);

	return 0;
}
