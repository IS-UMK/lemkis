#include "representation.hpp"
#include <print>

int main() {
    using namespace representation;

    fraction<int> my_fraction(2, 45);
    expansion<10> my_expansion = expansion<10>();

    std::print("\nLicznik:\t{}", my_fraction.numerator);
    std::print("\nMianownik:\t{}", my_fraction.denominator);
    std::print("\nBaza:\t{}", my_expansion.base_v);

    expand(my_fraction, my_expansion);
    std::print("\nWhole\t {}", my_expansion.whole);
    std::print("\nFriactial\t {}", my_expansion.fractial);
    std::print("\nPeriod\t {}", my_expansion.period);
    std::print("\n{}.{}({})", my_expansion.whole, my_expansion.fractial, my_expansion.period);

    return 0;
}
