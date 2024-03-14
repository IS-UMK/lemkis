#include <iostream>
#include <concepts>
#include <variant>
#include <concepts>
#include <expected>
#include <variant>
#include <vector>
#include <string>

namespace representation {

	struct Fraction {
		int numerator{ 0 };
		int denominator{ 1 };
	};

	struct Expansion {
		int base{ 10 };
	};

	class Rep {
	public:

		int load_numerator();

		int load_denominator();

		int load_base();

		Fraction create_fraction(int, int);

		Expansion create_expansion(int);

		std::string expansion_in_base(Fraction, Expansion);

	};

}