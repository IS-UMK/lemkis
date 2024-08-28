#include <iostream>
#include <algorithm>
#include <array>
#include <cstdint>
#include <print>
#include <format>
#include <vector>
#include <set>

import recursion;
import expect;


auto recursion_tests() -> bool{
    return testing::expect_equal(chocolates(15, 3, 3), 7) &&
            testing::expect_equal(is_prime(17), true) &&
            testing::expect_equal(product(15, 5), 75) &&
            testing::expect_equal(sum_of_digits(4523), 14) &&
            testing::expect_equal(to_binary(18), 10010) && 
            testing::expect_equal(iota_sum(20), 210) && 
            testing::expect_equal(tiles(5, 5), 25) && 
            testing::expect_equal(count_decompositions_as_sum_of_powers(26, 2), 2) && 
            testing::expect_equal(fibonacci(4), 3);
}


auto test_vectors() -> bool{
    std::vector<std::vector<std::size_t>> factorization{{2, 2, 2, 2}, {2, 2, 4}, {2, 8}, {4, 4}, {16}};
    std::vector<std::vector<std::size_t>> decompositions_sum{{1, 1, 1, 5}, {1, 1, 2, 4}, {1, 1, 3, 3}, {1, 2, 2, 3}, {2, 2, 2, 2}};
    std::vector<std::string> letter_sequences{"a", "aa", "ab", "ac", "b", "ba", "bb", "bc", "c", "ca", "cb", "cc"};
    std::vector<std::vector<std::size_t>> non_increasing{{6}, {5, 1}, {4, 2}, {4, 1, 1}, {3, 3}, {3, 2, 1}, {3, 1, 1, 1}, {2, 2, 2}, {2, 2, 1, 1}, {2, 1, 1, 1, 1}, {1, 1, 1, 1, 1, 1}};
    std::vector<int> representations_increased{1, 2, 3, 4, 5, 6, 7, 8, 9, 12, 13, 14, 15, 16, 17, 18, 19, 23, 24, 25, 26, 27, 28, 29, 34, 35, 36, 37, 38, 39, 45, 46, 47, 48, 49, 56, 57, 58, 59, 67, 68, 69, 78, 79, 89};
    std::vector<std::vector<int>> alternating_sequences{{1, 2, 3, 4, 5, 6}, {1, 2, 3, 4, 5}, {1, 2, 3, 4}, {1, 2, 3, 6}, {1, 2, 3}, {1, 2, 5, 6}, {1, 2, 5}, {1, 2}, {1, 4, 5, 6}, {1, 4, 5}, {1, 4}, {1, 6}, {1}, {3, 4, 5, 6}, {3, 4, 5}, {3, 4}, {3, 6}, {3}, {5, 6}, {5}};
    
    return testing::expect_equal(factorizations(16), factorization) &&
            testing::expect_equal(sum_decomposition(8, 4), decompositions_sum) &&
            testing::expect_equal(sequences_from_a_set(std::set<char>{'a', 'b', 'c'}, 2), letter_sequences) &&
            testing::expect_equal(non_increasing_decompositions(6), non_increasing) &&
            testing::expect_equal(increasing_representations(2), representations_increased) &&
            testing::expect_equal(alternating({1, 3, 5}, {2, 4, 6}), alternating_sequences);
}


auto test_sorts() -> bool{
    std::vector<std::int32_t> before_bubble_sort{3, 5, 1, 9, 15, 2};
    std::vector<std::int32_t> before_insertion_sort{3, 5, 1, 9, 15, 2};
    std::vector<std::int32_t> after_sort{1, 2, 3, 5, 9, 15};
    std::vector<std::int32_t> after_insertion_sort{1, 2, 3, 5, 9, 15};
    insertion_sort(before_insertion_sort);
    
    return testing::expect_equal(bubble_sort(before_bubble_sort), after_sort) &&
            testing::expect_equal(insertion_sort(before_insertion_sort), after_insertion_sort);
}


auto test_maze() -> bool{
    std::vector<bool> maze = {
        0, 1, 0, 1, 0,
        0, 1, 0, 1, 0,
        0, 0, 0, 0, 0,
        0, 1, 1, 1, 0,
        0, 0, 0, 1, 0,
    };
    return testing::expect_equal(path_in_maze(maze, 5, 5, 0, 0, 4, 4), true);
}


int main(){
    bool ok{testing::expect_equal(1, 1) &&
            testing::expect_equal(std::vector<std::int64_t>{1, 2},
                                  std::array{1, 2}) &&
            !testing::expect_equal(std::vector{1}, std::array{1, 2})};

    return std::ranges::all_of(std::array{ok,
                                          recursion_tests(),
                                          test_vectors(),
                                          test_sorts(),
                                          test_maze()},
                               std::identity{})
               ? 0
               : 1;
}