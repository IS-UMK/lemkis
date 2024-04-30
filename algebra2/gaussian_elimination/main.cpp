#include "gaussian_elimination.h"


int main() {
    gaussian_elimination_examples<int, std::layout_right>();
    gaussian_elimination_examples_fractions<int, std::layout_right>();
    return 0;
}