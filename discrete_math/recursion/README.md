# Recursive Solutions for Various Problems

This program implements several problems using recursive functions. Each solution is explained in C++ included in project files. This document provides descriptions of problems as well as their mathematical solutions.

# How to run 
You can run the code exclusively using the clang compiler. While in your build directory, use the following commands: 
`clang++-18 -std=gnu++26 -stdlib=libc++ -Wall -Wextra -Wpedantic -Werror <your_cpp_file> -o <your_output_file>`

## List of Problems

* `queens`
* `chocolates`
* `is_prime`
* `factorizations`
* `product`
* `sum_decomposition`
* `sum_of_digits`
* `to_binary`
* `iota_sum`
* `tiles`
* `generate_sequences`
* `sequences_from_a_set`
* `non_increasing_decompositions`
* `fibonacci`
* `path_in_maze`
* `subsets`
* `increasing_representations`
* `tower_of_hanoi`
* `more_ones`
* `count_decompositions_as_sum_of_powers`
* `bubble_sort`



# Mathematical Solutions

### 1. N-Queens Problem (`queens`)

#### Description
Place `n` queens on an `n x n` chessboard so that no two queens can attack each other. This means no two queens can be in the same row, column, or diagonal.

#### Mathematical Solution
The function `queens` explores all potential ways to position the queens on the board. It uses a helper function, `solve_n_queens_util`, which tries placing queens one column at a time and checks for conflicts with other queens. The function ensures that the queens are placed in safe positions by checking rows and diagonals.

### 2. Chocolates Problem (`chocolates`)

#### Description
Given a certain amount of money, buy as many chocolates as possible. Each chocolate has a price, and you can exchange a specific number of wrappers for additional chocolates.

#### Mathematical Solution
The function `chocolates` calculates how many chocolates you can initially buy with your money. Then, it uses a helper function, `getMaxChocolates`, to keep track of how many extra chocolates you can get by trading wrappers. This process repeats recursively until there aren't enough wrappers to trade for a new chocolate.

### 3. Prime Number Check (`is_prime`)

#### Description
Determine if a number is prime.

#### Mathematical Solution
The function `is_prime` checks if a number is prime by checking if it can be divided by any number other than 1 and itself. It utilizes a helper function, `is_prime_helper`, which tests divisors starting from 2 up to the square root of the number. If no divisors are found, the number is prime.

### 4. Factorizations (`factorizations`)

#### Description
Find all possible ways to factor a given number into smaller integers.

#### Mathematical Solution
The function `factorizations` finds all the different combinations of factors that multiply to the given number. The helper function `find_factors` breaks down the number into its factors and continues this process recursively until all factors are identified.

### 5. Product Using Addition (`product`)

#### Description
Calculate the product of two integers using only addition and recursion.

#### Mathematical Solution
The function `product` multiplies two numbers by adding one number to itself a specified number of times. It does this recursively by decreasing one of the numbers and adding the other number until the second number reaches zero.

### 6. Sum Decomposition (`sum_decomposition`)

#### Description
Given a number, find all possible ways to write it as a sum of integers.

#### Mathematical Solution
The function `sum_decomposition` generates all possible combinations of numbers that add up to the given number. The helper function `find_combinations` subtracts potential summands from the total and tracks the current combination until all valid combinations are found. 

### 7. Sum of Digits (`sum_of_digits`)

#### Description
Calculate the sum of the digits of a given number.

#### Mathematical Solution
The function `sum_of_digits` adds together the digits of a number by repeatedly extracting the last digit and summing it with the sum of the remaining digits. This process continues until the number is reduced to zero.

### 8. Binary Representation (`to_binary`)

#### Description
Convert a number to its binary representation.

#### Mathematical Solution
The function `to_binary` converts a number to binary by dividing the number by 2 and recording the remainders. This is done recursively, and the binary number is constructed from the remainders in reverse order.

### 9. Iota Sum (`iota_sum`)

#### Description
Find the sum of the first `n` natural numbers.

#### Mathematical Solution
The function `iota_sum` adds up the first `n` natural numbers by recursively adding each number to the sum of the numbers before it. The sum of the first `n` natural numbers can also be calculated using the formula `n * (n + 1) / 2`.

### 10. Tiles (`tiles`)

#### Description
Calculate the number of the largest power-of-2 tiles needed to completely cover a grid of given size.

#### Mathematical Solution
The `tiles` function uses the helper function `find_largest_tile` to find the largest power-of-2 tile that fits within the current grid dimensions. The grid is then divided into smaller sections by recursively applying the same process to the remaining uncovered portions until the entire grid is covered. Each step reduces the problem size by covering part of the grid with the largest possible tile.

### 12. Sequences from a Set (`sequences_from_a_set`)

#### Description
Create all possible sequences of letters from a given set with a length less than or equal to a maximum length.

#### Mathematical Solution
The function `sequences_from_a_set` generates all possible sequences by utilizing the helper function`generate_sequences` 
The `generate_sequences` function recursively builds sequences by adding each letter to the current sequence. It continues this process until the maximum length is reached or all letters are used up. The resulting sequences are stored in a result vector.

### 13. Non-Increasing Decompositions (`non_increasing_decompositions`)

#### Description
Generate all possible non-increasing decompositions of a given number.

#### Mathematical Solution
The function `non_increasing_decompositions` generates all sequences of non-increasing integers that sum to the given number. 

The helper function `generate_decompositions` performs the recursive decomposition by:
Checking if the remaining number is zero. If it is, the current sequence is added to the results. It also iterates through possible values for the next element in the sequence, starting from the minimum of `number` and `max_value`, down to 1. For each value: Adding the value to the current sequence, recursively calling `generate_decompositions` with the updated number (`number - i`) and the new maximum value (`i`). Removes the value from the current sequence after the recursive call completes (backtracking). The decompositions are stored in a result vector and are printed using the `print_decompositions` function.

### 14. Fibonacci (`fibonacci`)

#### Description
Finds the nth Fibonacci number.

#### Mathematical Solution
The Fibonacci sequence starts with 0 and 1. Each subsequent number in the sequence is the sum of the two earlier ones. For example, the sequence begins: 0, 1, 1, 2, 3, 5, 8, 13, 21, ...
To calculate the nth Fibonacci number, the `fibonacci` function uses recursion. It handles the base cases for the first two Fibonacci numbers (0 and 1) and then recursively calculates the Fibonacci number by adding the two previous Fibonacci numbers.

### 15. Path in Maze (`path_in_maze`)

#### Description
Given a maze (0 stands for a path, 1 for a wall) of shape `rows x columns`, checks if there is a path from point `(startx, starty)` to `(endx, endy)`. You can move in basic 4 directions (up, down, left, right).

#### Mathematical Solution
The function `path_in_maze` recursively checks for a valid path in the maze by exploring all possible movements. It uses backtracking to mark visited points and correctly determine if a path exists from the start point to the end point.

### 16. Subsets (`subsets`)

#### Description
Return all possible non-empty proper subsets of numbers.

#### Mathematical Solution
The helper function `generate_subsets` performs the recursive generation of subsets by iterating through the elements of the given set. For each element:
- It includes the element in the current subset.
- Recursively calls `generate_subsets` with the remaining elements, excluding the current one.
- It excludes the element from the current subset.
- Recursively calls `generate_subsets` with the remaining elements.

### 17. Increasing Representations (`increasing_representations`)

#### Description
Given the number of digits `number_of_digits` in a number, returns all `n`-digit numbers whose digits are strictly increasing from left to right. Leading zeros are allowed so for example, a two-digit `01` stands for number `1`.

#### Mathematical Solution
The function `increasing_representations` generates all `n`-digit numbers whose digits are strictly increasing from left to right. It uses recursion to generate numbers starting from 0 and ensures that the digits are strictly increasing.

### 18. Tower of Hanoi (`tower_of_hanoi`)

#### Description
Solve the Tower of Hanoi problem by presenting the steps involved in moving a tower of disks from one rod to another.

#### Mathematical Solution
The function `hanoi` moves `n` disks from the source rod to the destination rod:
- Move `n-1` disks from the source rod to the auxiliary rod.
- Move the nth disk from the source rod to the destination rod.
- Move the `n-1` disks from the auxiliary rod to the destination rod.
The `tower_of_hanoi` function initializes the process and collects the sequence of moves in a vector, which is then returned.

### 19. Generate Bit Representations (`more_ones`)

#### Description
Generate all possible bit representations of length `number_of_bits` with the condition that each prefix of the number contains more ones than zeros.

#### Mathematical Solution
The `more_ones` function generates possible bit representations recursively. It takes into account the number of ones and zeros added so far and the remaining bits to be added. The conditions for generating the bit representations are:
- If the number of remaining bits is zero, the current representation is added to the result vector.
- If adding a one does not violate the condition (onesCount + 1 >= zerosCount), a one is added to the current representation, and the function is called recursively.
- If adding a zero does not violate the condition (onesCount > zerosCount), a zero is added to the current representation, and the function is called recursively.
The resulting bit representations are stored in a vector and returned.

### 20. Count Decompositions as Sum of Powers (`count_decompositions_as_sum_of_powers`)

#### Description
Count all unique decompositions of a given number as the sum of powers of consecutive integers. 

#### Mathematical Solution
The `count_decompositions_as_sum_of_powers` function calculates the number of unique decompositions of a given number into the sum of powers of integers.
The function `count_decompositions` performs the recursive decomposition by iterating through potential bases starting from `currentBase`. It adds the current base to the decomposition and recursively calls itself with the updated parameters. After exploring all possibilities for the current base, it removes the last base from the decomposition and continues the search using new base.

### 21. Bubble Sort (`bubble_sort`)

#### Description
Sorts a given vector using the bubble sort algorithm and returns the sorted vector.

#### Mathematical Solution
The `bubble_pass` function is a helper function that performs a single pass of the bubble sort. It compares adjacent elements of the vector and swaps them if they are in the wrong order. This process is repeated recursively for `n-1` elements until the vector is sorted.


#### Authors
* Andrii Brilliant
* Radosław Gawryszewski
* Hubert Krakowiak
