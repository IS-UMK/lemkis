#include "representation.hpp"
#include <print>

int main() {
    using namespace representation_namespace;

    representation representation_class;

    fraction<int> my_fraction = representation_class.create_fraction(183, 23);
    expansion<16> my_expansion = representation_class.create_expansion<16>(16);

    std::print("\nLicznik:\t{}", my_fraction.numerator);
    std::print("\nMianownik:\t{}", my_fraction.denominator);
    std::print("\nBaza:\t{}", my_expansion.form);

    std::print("\nWynik zamiany:\t {}", representation_class.expansion_in_base(my_fraction, my_expansion));

    return 0;
}
