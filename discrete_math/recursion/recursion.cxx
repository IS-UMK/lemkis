module;
// NOLINTBEGIN
#include <iostream>
#include <print>
#include <set>
#include <string>
#include <vector>

export module recursion;
import recursion_helper;


/*
    description:
        Given n, of a n x n chessboard, finds the proper placement of queens on
        chessboard. Displays all possible solutions.
*/
export inline auto queens(int n) -> void {
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
        Solves the following problem. You have your_money money and you want to
   eat as many chocolates as possible. Price of one chocolate is
   price_of_chocolate. Additionally for wrap number of wrappers you can obtain
   one chocolate.
*/
export inline auto chocolates(std::size_t your_money,
                              std::size_t price_of_chocolate,
                              std::size_t wrap) -> std::size_t {
    const std::size_t initial_chocolates = your_money / price_of_chocolate;
    return initial_chocolates + get_max_chocolates(initial_chocolates, wrap);
}


/*
    description:
        Checks if a number is prime.
*/
export inline auto is_prime(std::size_t number) -> bool {
    if (number <= 1) { return false; }
    if (number == 2) { return true; }
    if (number % 2 == 0) { return false; }

    return is_prime_helper(number, 3);
}


/*
    description:
        Returns a vector of all possible factorizations of number.
*/
export inline auto factorizations(std::size_t number) {
    std::vector<std::vector<std::size_t>> all_factors;
    std::vector<std::size_t> current_factors;
    find_factors(number, 2, current_factors, all_factors);
    return all_factors;
}


/*
    description:
        Returns a * b using only `+` operation and recursion.
*/
export inline auto product(std::int32_t a, std::int32_t b) -> std::int64_t {
    if (b == 0) { return 0; }
    return a + product(a, b - 1);
}


/*
    description:
        Given number finds all possible solutions to x_1 + x_2 + ... +
*/
export inline auto sum_decomposition(std::size_t number,
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
export inline auto sum_of_digits(std::size_t number) -> std::size_t {
    const std::size_t reducer = 10;
    if (number == 0) { return 0; }

    return (number % reducer) + sum_of_digits(number / reducer);
}


/*
    description:
        Finds binary representation of a number.
*/
export inline auto to_binary(std::size_t number) -> std::size_t {
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
export inline auto iota_sum(std::uint32_t up_to) -> std::uint64_t {
    if (up_to == 0) { return 0; }
    return up_to + iota_sum(up_to - 1);
}


/*
    description:
        Given a area of size rows x columns find minimum number of tiles
        need to cover this area.
*/
export inline auto tiles(std::uint32_t rows,
                         std::uint32_t columns) -> std::uint32_t {
    if (rows == 0 || columns == 0) { return 0; }

    const std::uint32_t largest_tile =
        std::min(find_largest_tile(rows), find_largest_tile(columns));

    return 1 + tiles(rows - largest_tile, columns) +
           tiles(largest_tile, columns - largest_tile) +
           tiles(rows - largest_tile, largest_tile);
}


/*
    description:
        Solves the classical problem of tower of hanoi.
        Returns vector of made moves
*/
export inline auto tower_of_hanoi(std::size_t discs) {
    std::vector<std::string> moves;

    hanoi(discs, 'A', 'C', 'B', moves);

    return moves;
}


/*
    description:
        Given a set of letters create all possible sequences consisting of these
        letters (repetitions of a letter are permitted) of the
        length smaller or equal to maximal_length.
*/
export inline auto sequences_from_a_set(const std::set<char>& letters,
                                        std::uint8_t maximal_length)
    -> std::vector<std::string> {
    std::vector<std::string> result;
    generate_sequences(letters, "", maximal_length, result);
    return result;
}


/*
    description:
        Returns all possible number_of_bits bit representation of numbers n
        which satisfy the following condition: each prefix of n contains more
        ones than zeros.
*/
export inline auto more_ones(std::uint8_t number_of_bits)
    -> std::vector<std::string> {
    std::vector<std::string> result;
    generate_bit_representations("", 0, 0, number_of_bits, result);
    return result;
}


/*
    description:
        Counts all possible decomposition of number into sum of x's such that
   each x is of the form x = y ^ power for some y.
*/
export inline auto count_decompositions_as_sum_of_powers(double number,
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
        Returns all possible non-empty proper subsets of numbers.
*/
export inline auto subsets(std::set<std::int32_t> numbers) {
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
        Given number of digits number_of_digits in a number, returns all n-digit
        numbers whose digits are strictly increasing from left to right. Leading
        zeros are allowed so for example two digit 01 stands for number 1.
*/
export inline auto increasing_representations(std::uint8_t number_of_digits) {
    std::vector<int> results;
    generate_numbers(0, number_of_digits, -1, results);
    return results;
}


/*
    description:
        Given a number, returns all possible non-increasing sequences which sum
   up to number.
*/
export inline auto non_increasing_decompositions(std::size_t number) {
    std::vector<std::vector<std::size_t>> result;
    std::vector<std::size_t> current_sequence;
    generate_decompositions(number, number, current_sequence, result);
    return result;
}


/*
    description:
        Sorts v using bubble sort and returns sorted vector.
*/
export inline auto bubble_sort(std::vector<std::int32_t> v) {
    const std::size_t v_size = v.size();
    bubble_pass(v, v_size);
    return v;
}


/*
    description:
        Sorts v using insertion sort and returns sorted vector.
*/
export inline auto insertion_sort(std::vector<std::int32_t> v) {
    const std::size_t n = v.size();
    insertion_sort_recursive(v, n);
    return v;
}


/*
    description:
        Given two strictly increasing arrays v0 and v1 returns all strictly
   increasing sequences satisfying the following condition: a[2i] must be drawn
   from v0 and a[2i + 1] must be drawn from v1 for every integer i
*/
export inline auto alternating(const std::vector<int>& v0,
                               const std::vector<int>& v1) {
    std::vector<std::vector<int>> sequences;
    std::vector<int> current_sequence;

    find_sequences(v0, v1, 0, 0, current_sequence, sequences, true);

    return sequences;
}


/*
    description:
        Finds nth fibinacci number.
*/
export inline auto fibonacci(uint8_t nth) {
    if (nth == 0) { return 0; }
    if (nth == 1) { return 1; }
    return fibonacci(nth - 1) + fibonacci(nth - 2);
}


/*
    description:
        Given a maze (0 stands for a path, 1 for a wall) of shape rows x columns
        checks if there is a path from point (startx, starty) to (endx, endy).
*/
export inline auto path_in_maze(std::vector<bool> maze,
                                std::size_t rows,
                                std::size_t columns,
                                int startx,
                                int starty,
                                int endx,
                                int endy) -> bool {
    if (startx == endx && starty == endy) { return true; }
    if (!is_valid_move(maze, rows, columns, startx, starty)) { return false; }
    maze[startx * columns + starty] = true;
    const bool found_path =
        path_in_maze(maze, rows, columns, startx + 1, starty, endx, endy) ||
        path_in_maze(maze, rows, columns, startx - 1, starty, endx, endy) ||
        path_in_maze(maze, rows, columns, startx, starty + 1, endx, endy) ||
        path_in_maze(maze, rows, columns, startx, starty - 1, endx, endy);
    maze[startx * columns + starty] = false;
    return found_path;
}
// NOLINTEND
