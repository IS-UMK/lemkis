// NOLINTBEGIN
module;
#include <iostream>
#include <print>
#include <set>
#include <string>
#include <vector>

export module recursion_helper;


/*
    description:
        Helper function to check if queen's position is safe on the chessboard
*/
export inline auto is_safe(const std::vector<int>& board,
                           int row,
                           int col) -> bool {
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
export inline void solve_n_queens_util(
    int n,
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
export inline auto print_solutions_helper(int n, int i, auto solution) {
    for (int j = 0; j < n; j++) {
        solution[j] == i ? std::print("Q ") : std::print(". ");
    }
}


/*
    description:
        Prints out solutions for queens
*/
export inline auto print_solutions(
    const std::vector<std::vector<int>>& solutions,
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
        Calculates max chocolates that you can buy and trade for wrappers.
*/
export inline auto get_max_chocolates(std::size_t wrappers,
                                      std::size_t wrap) -> std::size_t {
    if (wrappers < wrap) { return 0; }

    const std::size_t new_chocolates = wrappers / wrap;
    const std::size_t remaining_wrappers = wrappers % wrap;

    return new_chocolates +
           get_max_chocolates(new_chocolates + remaining_wrappers, wrap);
}


/*
    description:
       Helper function to check if a number is prime.
*/
export inline auto is_prime_helper(std::size_t number,
                                   std::size_t divisor) -> bool {
    if (divisor * divisor > number) { return true; }
    if (number % divisor == 0) { return false; }

    if (divisor == 2) { return is_prime_helper(number, 3); }
    return is_prime_helper(number, divisor + 2);
}


/*
    description:
        Helper function to find all possible factorizations of a number.
*/
export inline auto find_factors(
    std::size_t number,
    std::size_t start,
    std::vector<std::size_t>& current_factors,
    std::vector<std::vector<std::size_t>>& all_factors) -> void {
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
        Helper function to find all possible solutions to x_1 + x_2 + ... +
*/
export inline auto find_combinations(
    std::size_t number,
    std::uint8_t number_of_factors,
    std::size_t start,
    std::vector<std::size_t>& current_combination,
    std::vector<std::vector<std::size_t>>& all_combinations) -> void {
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
        Helper function to find the largest possible tile.
*/
export inline auto find_largest_tile(std::uint32_t size) -> std::uint32_t {
    std::uint32_t tile_size = 1;
    while (tile_size * 2 <= size) { tile_size *= 2; }
    return tile_size;
}


/*
    description:
        Helper function to solve the classical problem
        of tower of hanoi.
*/
export inline auto hanoi(std::size_t n,
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
        Helper function to create all possible sequences consisting of given
   letters.
*/
export inline auto generate_sequences(const std::set<char>& letters,
                                      const std::string& current_sequence,
                                      std::size_t maxLength,
                                      std::vector<std::string>& result)
    -> void {
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
        Helper function to find all possible bit representations
        for number_of_bits given by function more_ones.
*/
export inline auto generate_bit_representations(
    const std::string& current,
    int onesCount,
    int zerosCount,
    int remainingBits,
    std::vector<std::string>& result) -> void {
    if (remainingBits == 0) {
        result.push_back(current);
        return;
    }
    if (onesCount + 1 >= zerosCount) {
        generate_bit_representations(current + '1',
                                     onesCount + 1,
                                     zerosCount,
                                     remainingBits - 1,
                                     result);
    }
    if (onesCount > zerosCount) {
        generate_bit_representations(current + '0',
                                     onesCount,
                                     zerosCount + 1,
                                     remainingBits - 1,
                                     result);
    }
}


/*
    description:
        Helper function to count all possible decomposition of number i with
   condition given in count_decomposition_as_sum_of_powers.
*/
export inline auto count_decompositions(double num,
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
        Helper function to find all possible subsets of numbers.
*/
export inline auto generate_subsets(const std::set<std::int32_t>& numbers,
                                    const std::int32_t* num_array,
                                    std::size_t index,
                                    std::vector<int>& subset,
                                    std::vector<std::vector<int>>& subsets)
    -> void {
    if (!subset.empty()) { subsets.push_back(subset); }
    for (std::size_t i = index; i < numbers.size(); ++i) {
        subset.push_back(num_array[i]);
        generate_subsets(numbers, num_array, i + 1, subset, subsets);
        subset.pop_back();
    }
}


/*
    description:
        Helper function to find all n-digit numbers
        whose digits are strictly increasing from left to right.
*/
export inline auto generate_numbers(int currentNumber,
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
        Helper function to find all non-increasing decompositions of a number.
*/
export inline auto generate_decompositions(
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
        Helper function to sort a vector using bubble sort
*/
export inline auto bubble_pass(std::vector<int>& v, std::size_t n) -> void {
    if (n <= 1) { return; }
    std::size_t count = 0;
    for (std::size_t i = 0; i < n - 1; ++i) {
        if (v[i] > v[i + 1]) {
            std::swap(v[i], v[i + 1]);
            count++;
        }
    }
    if (count == 0) { return; };
    bubble_pass(v, n - 1);
}


/*
    description:
        Helper function to sort a vector using insertion sort
*/
export inline auto insertion_sort_recursive(std::vector<std::int32_t>& v,
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
        Checks if the move in the maze is valid .
*/
export inline auto is_valid_move(const std::vector<bool>& maze,
                                 std::size_t rows,
                                 std::size_t columns,
                                 std::size_t x,
                                 std::size_t y) -> bool {
    return x >= 0 && x < rows && y >= 0 && y < columns &&
           !maze[x * columns + y];
}


export inline auto find_sequences(const std::vector<int>& v0,
                                  const std::vector<int>& v1,
                                  int indexi,
                                  int indexj,
                                  std::vector<int>& current_sequence,
                                  std::vector<std::vector<int>>& sequences,
                                  bool fromV0)
    -> std::vector<std::vector<int>> {
    if (fromV0) {
        for (int i = indexi; i < v0.size(); ++i) {
            if (current_sequence.empty() || v0[i] > current_sequence.back()) {
                current_sequence.push_back(v0[i]);
                find_sequences(
                    v0, v1, i + 1, indexj, current_sequence, sequences, false);
                current_sequence.pop_back();
            }
        }
    } else {
        for (int j = indexj; j < v1.size(); ++j) {
            if (v1[j] > current_sequence.back()) {
                current_sequence.push_back(v1[j]);
                find_sequences(
                    v0, v1, indexi, j + 1, current_sequence, sequences, true);
                current_sequence.pop_back();
            }
        }
    }

    if (!current_sequence.empty()) { sequences.push_back(current_sequence); }

    return sequences;
}
// NOLINTEND
