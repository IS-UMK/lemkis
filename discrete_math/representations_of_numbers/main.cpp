#include <iostream>
#include "representation.hpp"

int main(){
    using namespace representation;

    representation::fraction<int> my_fraction{ 64, 30 };
    representation::expansion<10> expand = representation::expand<10>(my_fraction);
    std::cout << std::format("Expansion format: {}", expand);
    

    representation::expansion<10> expan{ "1", "0", "1" };
    representation::fraction<int> fraction2 = representation::dexpand<10, int>(expan);
    std::cout << std::format("\nFraction format: {}", fraction2);

    return 0;
}
