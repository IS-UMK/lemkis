# Project Description

Our project allows end-users to perform Gaussian elimination on matrices of different sizes. It's based on three basic algorithms: swapping two rows, multiplying a row by a nonzero number, and adding a multiple of one row to another row.

## How to Run the Code
To compile the code on your computer, navigate to the build directory and use the following command:

clang++-18 -stdlib=libc++ -std=c++23 ./main.cpp -o main

**NOTE**: Ensure that the files `gaussian_elimination.hpp` and `matrix.hpp` are in the same directory.

## Functions

- `fraction` struct representing fractions.
- `run()` utilizes the three operations mentioned in the Project Description.
- `show_steps()` prints out the matrix between using two operations.
- `determinant()` calculates the determinant of the matrix if the matrix is square.
- `inverse()` calculates the inverse matrix of matrix `m` if the matrix is square and invertible.

You can run `examples()` to understand the implementation of each function.

## Questions
If there's a part of the code not mentioned in this document that needs explanation, please refer to the source code or ask the authors.

## Authors
- Andrii Bryliant
- Alan Czerski
- Radosław Gawryszewski
