#include "./euclidean.hpp"
#include <cassert>


int main() {

    //Tests to see if code is working
    for (int i = 0; i < 10; i++)
    {
        for (int j = 0; j < 10; j++)
        {
            assert(algorithms::test(i, j));
        }
    }
    assert(algorithms::test(2, 4, 6, 2));
    assert(algorithms::test(546, 308, 70, 14));

    algorithms::example_gcd_for_two_numbers(16, 8);
    algorithms::example_gcd_for_n_numbers(546, 308, 70);

    return 0;
}