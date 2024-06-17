# Project Description

Our project allows end-users to perform Gaussian elimination on matrices of different sizes. It's based on three basic algorithms: swapping two rows, multiplying a row by a nonzero number, and adding a multiple of one row to another row.

## How to Run the Code
To compile the code on your computer, navigate to the build directory and use the following command:

clang++-18 -std=gnu++26 -stdlib=libc++ -Wall -Wextra -Wpedantic -Werror <your_cpp_file> -o <your_output_file>


**NOTE**: Ensure that you have both projects `gaussian_elimination` and `matrix` on your local computer as this project depends on `matrix` project

## Functions

- `fraction` struct representing fractions.
- `run()` utilizes the three operations mentioned in the Project Description.
- `show_steps()` prints out the matrix between using two operations.
- `determinant()` calculates the determinant of the matrix if the matrix is square.
- `inverse()` calculates the inverse matrix of matrix `m` if the matrix is square and invertible.

You can run `examples_run_and_steps()` or `examples_determinant_and_inverse()` in `tests.hpp` file to understand the implementation of each function.

And by running `run_test()`, `determinant_test()`, `inverse_test()`, `column_layout_test()` you can make sure that everything works and behaves how it should.

## Questions
If there's a part of the code not mentioned in this document that needs explanation, please refer to the source code or ask the authors.

## Authors
- Andrii Bryliant
- Alan Czerski
- Radosław Gawryszewski
