#pragma once

#include "recursion.hpp"

/*
    description:
        First part of recursion examples.
*/
inline auto recursion_examples(){
    std::println("\n////////Recursive functions examples////////\n");
    std::println("All possible proper positions of queens on a 4x4 chessboard: ");
    queens(4);
    std::println("How much chocolates if you have 10 money, the price of one chocolate is 2 "
                 "and for 2 wrappers you can get 1 chocolate: {}\n", chocolates(10, 2, 2));
    const int num = 17;
    if(is_prime(num)){
        std::println("{} is a prime number\n", num);
    }else{
        std::println("{} is not a prime number\n", num);
    }
    std::println("All possible factorizations of number 16: {}\n", factorizations(16));
    std::println("The product of 15 and 5 using recursion is {}\n", product(15, 5));
    std::println("The sum decomposition of number 14 using 4 factors is {}\n", sum_decomposition(14, 4));
    std::println("The sum of digits of nubmer 156764 is {}\n", sum_of_digits(156764));
}


/*
    description:
        Second part of recursion examples.
*/
inline auto recursion_examples_continued(){
    std::println("\n////////Recursive functions examples////////\n");
    std::println("Binary representation of number 19 is {}\n", to_binary(19));
    std::println("The sum of 1 + 2 + 3 + .. + 27 is {}\n", iota_sum(27));
    std::println("Minimun amount of tiles needed to cover a 7 x 8 board is {}\n", tiles(7, 8));
    std::set<char> letters{'a', 'b', 'c', 'd', 'e'};
    std::println("Number of all possible decomposition of number 26 into sum of x's such that each x "
                "is of the form x = y ^ 2: {}\n", count_decompositions_as_sum_of_powers(26, 2));
    std::println("All the possible sequences of letters using letters a b c d with max length of 2 are: {}\n", sequences_from_a_set(letters, 2));
    std::println("All possible non increasing decompositions of number 10: {}\n", non_increasing_decompositions(10));
    std::println("All possible increasing combinations of digits with max length of 3: {}\n", increasing_representations(3));                
}


/*
    description:
        Second part of recursion examples.
*/
inline auto recursion_examples_final(){
    std::println("\n////////Recursive functions examples////////\n");
    std::vector<int> v{1, 6, 3, 4, 9, 2, 5};
    std::vector<int> v1{1, 4, 3, 5, 19, 31, 14, 13};
    std::vector<bool> maze = {
        0, 1, 0, 1, 0,
        0, 1, 0, 1, 0,
        0, 0, 0, 1, 0,
        0, 1, 1, 1, 0,
        0, 0, 0, 1, 0,
    };
    std::println("Vector {} sorted using bubble sort: {}\n", v, bubble_sort(v));
    std::println("Vector {} sorted using insertion sort: {}\n", v1, insertion_sort(v1));
    std::println("The 9th Fibonacci number is {}\n", fibonacci(9));
    if(path_in_maze(maze, 5, 5, 0, 0, 4, 4)){
        std::println("There is no path to coordinates (4, 4) from (0, 0) in given maze\n");
    }else{
        std::println("There is a path to coordinates (4, 4) from (0, 0) in given maze\n", maze);
    }
}
