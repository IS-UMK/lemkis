#include "representation.hpp"

int main() {
    using namespace representation_namespace;

    representation representation_class;

    fraction<int> my_fraction = representation_class.create_fraction(183, 23);
    expansion<16> my_expansion = representation_class.create_expansion<16>(16);

    std::cout << "\nLicznik:\t" << my_fraction.numerator;
    std::cout << "\nMianownik:\t" << my_fraction.denominator;
    std::cout << "\nBaza:\t" << my_expansion.form;
    std::cout << "\nWynik zamiany:\t" << representation_class.expansion_in_base(my_fraction, my_expansion);

    return 0;
}
