#pragma once

#include <iostream>
#include <print>
#include <set>
#include <string>
#include <vector>
#include <cstdint>
#include <print>
#include <cmath>


/*
    description:
        Helper function to check if queen's position is safe on the chessboard
*/
inline auto is_safe(const std::vector<int>& board, int row, int col) -> bool {
    for (int i = 0; i < col; i++) {
        if (board[i] == row || abs(board[i] - row) == abs(i - col)) {
            return false;
        }
    }
    return true;
}


/*
    description:
        Helper recursive function for queens to find all possible solutions of
        queens on a n x n chessboard
*/
inline void solve_n_queens_util(int n,
                                int col,
                                std::vector<int>& board,
                                std::vector<std::vector<int>>& solutions) {
    if (col == n) {
        solutions.push_back(board);
        return;
    }

    for (int i = 0; i < n; i++) {
        if (is_safe(board, i, col)) {
            board[col] = i;
            solve_n_queens_util(n, col + 1, board, solutions);
        }
    }
}


/*
    description:
        Helper function for print_solutions
*/
inline auto print_solutions_helper(int n,
                                   int i,
                                   auto solution) {
    for (int j = 0; j < n; j++) {
        solution[j] == i ? std::print("Q ") : std::print(". ");
    }
}


/*
    description:
        Prints out solutions for queens
*/
inline auto print_solutions(const std::vector<std::vector<int>>& solutions,
                            int n) {
    for (const std::vector<int>& solution : solutions) {
        for (int i = 0; i < n; i++) {
            print_solutions_helper(n, i, solution);
            std::print("\n");
        }
        std::print("\n");
    }
}


/*
    description:
        Given n, of a n x n chessboard, finds the proper placement of queens on
        chessboard. Displays all possible solutions.
*/
inline auto queens(int n) -> void {
    if (n == 2 || n == 3) {
        std::print("No solutions for n = {}\n", n);
        return;
    }

    std::vector<std::vector<int>> solutions;
    std::vector<int> board(n, -1);

    solve_n_queens_util(n, 0, board, solutions);
    print_solutions(solutions, n);
}


/*
    description:
        Calculates max chocolates that you can buy and trade for wrappers.
*/
inline auto get_max_chocolates(std::size_t wrappers, std::size_t wrap)
    -> std::size_t {
    if (wrappers < wrap) { return 0; }

    const std::size_t new_chocolates = wrappers / wrap;
    const std::size_t remaining_wrappers = wrappers % wrap;

    return new_chocolates +
           get_max_chocolates(new_chocolates + remaining_wrappers, wrap);
}


/*
    description:
        Solves the following problem. You have your_money money and you want to eat
        as many chocolates as possible. Price of one chocolate is price_of_chocolate.
        Additionally for wrap number of wrappers you can obtain one chocolate.
*/
inline auto chocolates(std::size_t your_money,
                       std::size_t price_of_chocolate,
                       std::size_t wrap) -> std::size_t {
    const std::size_t initial_chocolates = your_money / price_of_chocolate;
    return initial_chocolates + get_max_chocolates(initial_chocolates, wrap);
}


/*
    description:
       Helper function to check if a number is prime.
*/
inline auto is_prime_helper(std::size_t number, std::size_t divisor) -> bool {
    if (divisor * divisor > number) { return true; }
    if (number % divisor == 0) { return false; }

    if (divisor == 2) { return is_prime_helper(number, 3); }
    return is_prime_helper(number, divisor + 2);
}


/*
    description:
        Checks if a number is prime.
*/
inline auto is_prime(std::size_t number) -> bool {
    if (number <= 1) { return false; }
    if (number == 2) { return true; }
    if (number % 2 == 0) { return false; }

    return is_prime_helper(number, 3);
}


/*
    description:
        Helper function to find all possible factorizations of a number.
*/
inline auto find_factors(std::size_t number,
                         std::size_t start,
                         std::vector<std::size_t>& current_factors,
                         std::vector<std::vector<std::size_t>>& all_factors)
    -> void {
    if (number == 1) {
        if (!current_factors.empty()) {
            all_factors.push_back(current_factors);
        }
        return;
    }

    for (std::size_t i = start; i <= number; ++i) {
        if (number % i == 0) {
            current_factors.push_back(i);
            find_factors(number / i, i, current_factors, all_factors);
            current_factors.pop_back();
        }
    }
}


/*
    description:
        Returns a vector of all possible factorizations of number.
*/
inline auto factorizations(std::size_t number) {
    std::vector<std::vector<std::size_t>> all_factors;
    std::vector<std::size_t> current_factors;
    find_factors(number, 2, current_factors, all_factors);
    return all_factors;
}


/*
    description:
        Returns a * b using only `+` operation and recursion.
*/
inline auto product(std::int32_t a, std::int32_t b) -> std::int64_t {
    if (b == 0) { return 0; }
    return a + product(a, b - 1);
}


/*
    description:
        Helper function to find all possible solutions to x_1 + x_2 + ... +
*/
inline auto find_combinations(std::size_t number,
                              std::uint8_t number_of_factors,
                              std::size_t start,
                              std::vector<std::size_t>& current_combination,
                              std::vector<std::vector<std::size_t>>& all_combinations)
    -> void {
    if (number_of_factors == 0 && number == 0) {
        all_combinations.push_back(current_combination);
        return;
    }
    if (number_of_factors == 0 || number == 0) { return; }
    for (std::size_t i = start; i <= number; ++i) {
        current_combination.push_back(i);
        find_combinations(number - i,
                          number_of_factors - 1,
                          i,
                          current_combination,
                          all_combinations);
        current_combination.pop_back();
    }
}


/*
    description:
        Given number finds all possible solutions to x_1 + x_2 + ... +
*/
inline auto sum_decomposition(std::size_t number,
                              std::uint8_t number_of_factors) {
    std::vector<std::vector<std::size_t>> all_combinations;
    std::vector<std::size_t> current_combination;
    find_combinations(
        number, number_of_factors, 1, current_combination, all_combinations);
    return all_combinations;
}


/*
    description:
        Returns sum of digits of number
*/
inline auto sum_of_digits(std::size_t number) -> std::size_t {
    const std::size_t reducer = 10;
    if (number == 0) { return 0; }

    return (number % reducer) + sum_of_digits(number / reducer);
}


/*
    description:
        Finds binary representation of a number.
*/
inline auto to_binary(std::size_t number) -> std::size_t {
    const std::size_t multiplier = 10;
    if (number == 0) { return 0; }

    const std::size_t binary = to_binary(number / 2);

    const std::size_t remainder = number % 2;
    return binary * multiplier + remainder;
}


/*
    description:
        Returns 1 + 2 + ... + up_to.
*/
inline auto iota_sum(std::uint32_t up_to) -> std::uint64_t {
    if (up_to == 0) { return 0; }
    return up_to + iota_sum(up_to - 1);
}


/*
    description:
        Helper function to find the largest possible tile.
*/
inline auto find_largest_tile(std::uint32_t size) -> std::uint32_t {
    std::uint32_t tile_size = 1;
    while (tile_size * 2 <= size) { tile_size *= 2; }
    return tile_size;
}


/*
    description:
        Given a area of size rows x columns find minimum number of tiles
        need to cover this area.
*/
inline auto tiles(std::uint32_t rows, std::uint32_t columns) -> std::uint32_t {
    if (rows == 0 || columns == 0) { return 0; }

    const std::uint32_t largest_tile =
        std::min(find_largest_tile(rows), find_largest_tile(columns));

    return 1 + tiles(rows - largest_tile, columns) +
           tiles(largest_tile, columns - largest_tile) +
           tiles(rows - largest_tile, largest_tile);
}


/*
    description:
        Helper function to solve the classical problem
        of tower of hanoi.
*/
inline auto hanoi(std::size_t n,
                  char source,
                  char destination,
                  char auxiliary,
                  std::vector<std::string>& moves) {
    if (n == 1) {
        moves.push_back("Move disk 1 from " + std::string(1, source) + " to " +
                        std::string(1, destination));
        return;
    }

    hanoi(n - 1, source, auxiliary, destination, moves);
    moves.push_back("Move disk " + std::to_string(n) + " from " +
                    std::string(1, source) + " to " +
                    std::string(1, destination));
    hanoi(n - 1, auxiliary, destination, source, moves);
}


/*
    description:
        Solves the classical problem of tower of hanoi.
        Returns vector of made moves
*/
inline auto tower_of_hanoi(std::size_t discs) {
    std::vector<std::string> moves;

    hanoi(discs, 'A', 'C', 'B', moves);

    return moves;
}


/*
    description:
        Helper function to create all possible sequences consisting of given letters.
*/
inline auto generate_sequences(const std::set<char>& letters,
                               const std::string& current_sequence,
                               std::size_t maxLength,
                               std::vector<std::string>& result) -> void {
    if (!current_sequence.empty() && current_sequence.length() <= maxLength) {
        result.push_back(current_sequence);
    }
    if (current_sequence.length() == maxLength) { return; }
    for (const char letter : letters) {
        generate_sequences(
            letters, current_sequence + letter, maxLength, result);
    }
}


/*
    description:
        Given a set of letters create all possible sequences consisting of these
        letters (repetitions of a letter are permitted) of the
        length smaller or equal to maximal_length.
*/
inline auto sequences_from_a_set(const std::set<char>& letters,
                                 std::uint8_t maximal_length)
    -> std::vector<std::string> {
    std::vector<std::string> result;
    generate_sequences(letters, "", maximal_length, result);
    return result;
}


/*
    description:
        Helper function to find all possible bit representations
        for number_of_bits given by function more_ones.
*/
inline auto generate_bit_representations(const std::string& current,
                                         int onesCount,
                                         int zerosCount,
                                         int remainingBits,
                                         std::vector<std::string>& result)
    -> void {
    if (remainingBits == 0) {
        result.push_back(current);
        return;
    }
    if (onesCount + 1 >= zerosCount) {
        generate_bit_representations(current + '1', onesCount + 1, zerosCount, remainingBits - 1, result);
    }
    if (onesCount > zerosCount) {
        generate_bit_representations(current + '0', onesCount, zerosCount + 1, remainingBits - 1, result);
    }
}


/*
    description:
        Returns all possible number_of_bits bit representation of numbers n
        which satisfy the following condition: each prefix of n contains more
        ones than zeros.
*/
inline auto more_ones(std::uint8_t number_of_bits) -> std::vector<std::string> {
    std::vector<std::string> result;
    generate_bit_representations("", 0, 0, number_of_bits, result);
    return result;
}


/*
    description:
        Helper function to count all possible decomposition of number i with condition
        given in count_decomposition_as_sum_of_powers.
*/
inline auto count_decompositions(double num,
                                 int pow,
                                 int currentBase,
                                 std::set<std::vector<int>>& decomps,
                                 std::vector<int>& current) {
    if (num == 0) {
        decomps.insert(current);
        return;
    }
    for (int base = currentBase; std::pow(base, pow) <= num; ++base) {
        current.push_back(base);
        count_decompositions(
            num - std::pow(base, pow), pow, base + 1, decomps, current);
        current.pop_back();
    }
}


/*
    description:
        Counts all possible decomposition of number into sum of x's such that each x
        is of the form x = y ^ power for some y.
*/
inline auto count_decompositions_as_sum_of_powers(double number,
                                                  std::uint8_t power)
    -> std::size_t {
    std::set<std::vector<int>> decompositions;
    std::vector<int> current_decomposition;

    count_decompositions(
        number, power, 1, decompositions, current_decomposition);

    return decompositions.size();
}


/*
    description:
        Helper function to find all possible subsets of numbers.
*/
inline auto generate_subsets(const std::set<std::int32_t>& numbers,
                             const std::int32_t* num_array,
                             std::size_t index,
                             std::vector<int>& subset,
                             std::vector<std::vector<int>>& subsets) -> void {
    if (!subset.empty()) { subsets.push_back(subset); }
    for (std::size_t i = index; i < numbers.size(); ++i) {
        subset.push_back(num_array[i]);
        generate_subsets(numbers, num_array, i + 1, subset, subsets);
        subset.pop_back();
    }
}


/*
    description:
        Returns all possible non-empty proper subsets of numbers.
*/
inline auto subsets(std::set<std::int32_t> numbers) {
    std::vector<std::vector<int>> result;
    std::vector<int> subset;
    const std::size_t num_size = numbers.size();
    std::int32_t* num_array;
    num_array = new (std::nothrow) std::int32_t[num_size];
    std::copy(numbers.begin(), numbers.end(), num_array);
    generate_subsets(numbers, num_array, 0, subset, result);
    return result;
}


/*
    description:
        Helper function to find all n-digit numbers 
        whose digits are strictly increasing from left to right.
*/
inline auto generate_numbers(int currentNumber,
                             int num_digits,
                             int last_digit,
                             std::vector<int>& results) -> void {
    const int digit_nine = 9;
    const int digit_ten = 10;
    if (num_digits == 0) {
        results.push_back(currentNumber);
        return;
    }
    for (int next_digit = last_digit + 1; next_digit <= digit_nine;
         ++next_digit) {
        generate_numbers(currentNumber * digit_ten + next_digit,
                         num_digits - 1,
                         next_digit,
                         results);
    }
}

/*
    description:
        Given number of digits number_of_digits in a number, returns all n-digit
        numbers whose digits are strictly increasing from left to right. Leading
        zeros are allowed so for example two digit 01 stands for number 1.
*/
inline auto increasing_representations(std::uint8_t number_of_digits) {
    std::vector<int> results;
    generate_numbers(0,
                     number_of_digits,
                     -1,
                     results);
    return results;
}


/*
    description:
        Helper function to find all non-increasing decompositions of a number.
*/
inline auto generate_decompositions(
    std::size_t number,
    std::size_t max_value,
    std::vector<std::size_t>& current_sequence,
    std::vector<std::vector<std::size_t>>& result) -> void {
    if (number == 0) {
        result.push_back(current_sequence);
        return;
    }

    for (std::size_t i = std::min(max_value, number); i > 0; --i) {
        current_sequence.push_back(i);
        generate_decompositions(number - i, i, current_sequence, result);
        current_sequence.pop_back();
    }
}


/*
    description:
        Given a number, returns all possible non-increasing sequences which sum up to
        number.
*/
inline auto non_increasing_decompositions(std::size_t number){
    std::vector<std::vector<std::size_t>> result;
    std::vector<std::size_t> current_sequence;
    generate_decompositions(number, number, current_sequence, result);
    return result;
}


/*
    description:
        Helper function to sort a vector using bubble sort
*/
inline auto bubble_pass(std::vector<int>& v, std::size_t n) -> void {
    if (n <= 1) { return; }

    for (std::size_t i = 0; i < n - 1; ++i) {
        if (v[i] > v[i + 1]) { std::swap(v[i], v[i + 1]); }
    }

    bubble_pass(v, n - 1);
}


/*
    description:
        Sorts v using bubble sort and returns sorted vector.
*/
inline auto bubble_sort(std::vector<std::int32_t> v) {
    const std::size_t v_size = v.size();
    bubble_pass(v, v_size);
    return v;
}


/*
    description:
        Helper function to sort a vector using insertion sort
*/
inline auto insertion_sort_recursive(std::vector<std::int32_t>& v,
                                     std::size_t n) -> void {
    if (n <= 1) { return; }
    insertion_sort_recursive(v, n - 1);
    const std::int32_t last = v[n - 1];
    std::size_t j = n - 2;
    while (j >= 0 && v[j] > last) {
        v[j + 1] = v[j];
        j--;
    }
    v[j + 1] = last;
}


/*
    description:
        Sorts v using insertion sort and returns sorted vector.
*/
inline auto insertion_sort(std::vector<std::int32_t> v) {
    const std::size_t n = v.size();
    insertion_sort_recursive(v, n);
    return v;
}




// NOLINTBEGIN
/*
    description:
        Recursive function to generate strictly increasing sequences.
*/
inline auto generate_sequences(const std::vector<int>& v0, const
std::vector<int>& v1, std::vector<int>& current_sequence,
std::vector<std::vector<int>>& sequences, std::size_t index) -> void {
    if (index == current_sequence.size()) {
        sequences.push_back(current_sequence);
        return;
    }
    if (index % 2 == 0) { // If the index is even, choose from v0
        for (const int num : v0) {
            if (num > current_sequence.back()) {
                current_sequence[index] = num;
                generate_sequences(v0, v1, current_sequence, sequences, index
                + 1);
            }
        }
    } else {
        for (int num : v1) {
            if (num > current_sequence.back()) {
                current_sequence[index] = num;
                generate_sequences(v0, v1, current_sequence, sequences, index
                + 1);
            }
        }
    }
}


/*
    description:
        Given two strictly increasing arrays v0 and v1 returns all strictly increasing
        sequences satisfying the following condition: a[2i] must be drawn from v0 and
        a[2i + 1] must be drawn from v1 for every integer i
*/
inline auto alternating(const std::vector<int>& v0, const
std::vector<int>& v1) {
    std::vector<std::vector<int>> sequences;
    std::vector<int> current_sequence((v0.size() + v1.size()), 0);
    current_sequence[0] = v0[0];

    generate_sequences(v0, v1, current_sequence, sequences, 1);
    return sequences;
}
// NOLINTEND


/*
    description:
        Finds nth fibinacci number.
*/
inline auto fibonacci(uint8_t nth) {
    if (nth == 0) { return 0; }
    if (nth == 1) { return 1; }
    return fibonacci(nth - 1) + fibonacci(nth - 2);
}


/*
    description:
        Checks if the move in the maze is valid .
*/
inline auto is_valid_move(const std::vector<bool>& maze,
                          std::size_t rows,
                          std::size_t columns,
                          std::size_t x,
                          std::size_t y) -> bool {
    return x >= 0 && x < rows && y >= 0 && y < columns &&
           !maze[x * columns + y];
}


/*
    description:
        Given a maze (0 stands for a path, 1 for a wall) of shape rows x columns
        checks if there is a path from point (startx, starty) to (endx, endy).
*/
inline auto path_in_maze(std::vector<bool> maze,
                         std::size_t rows,
                         std::size_t columns,
                         int startx,
                         int starty,
                         int endx,
                         int endy) -> bool {
    if (startx == endx && starty == endy) {
        return true;
    }
    if (!is_valid_move(maze, rows, columns, startx, starty)) {
        return false; 
    }
    maze[startx * columns + starty] =
        true;
    const bool found_path = path_in_maze(maze, rows, columns, startx + 1, starty, endx, endy) ||
                            path_in_maze(maze, rows, columns, startx - 1, starty, endx, endy) ||
                            path_in_maze(maze, rows, columns, startx, starty + 1, endx, endy) ||
                            path_in_maze(maze, rows, columns, startx, starty - 1, endx, endy);
    maze[startx * columns + starty] = false;
    return found_path;
}