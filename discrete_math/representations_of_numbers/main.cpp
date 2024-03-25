#include "representation.hpp"
#include <print>

int main() {
    using namespace representation;

    fraction<int> my_fraction(2, 45);
    expansion<10> my_expansion = expansion<10>();

    my_expansion = expand(my_fraction, my_expansion);
    std::print("\nUłamek: {}.{}({})", my_expansion.whole, my_expansion.fractial, my_expansion.period);

    expansion<10> my_expansion2 = expansion<10>("72", "0", "4");
    fraction<int> my_fraction2 = fraction<int>();
    my_fraction2 = dexpand(my_fraction2, my_expansion2);
    std::print("\nLicznik: {}\nMianownik: {}", my_fraction2.numerator, my_fraction2.denominator);

    return 0;
}
