#include "./euclidean.hpp"
#include <print>
#include <iostream>
#include <array>
#include <fstream>
#include <numeric>
#include <vector>
#include <cctype>
#include <cassert>


std::string ExampleForTwoDigits(int digit1, int digit2)
{
    auto [result, numbers] = algorithms::gcd_extended(digit1, digit2);
    auto [coefficients, gcd] = result;
    auto [x, y] = coefficients;
    std::println("\n\n\nExample1:\nStarting with {}, {}, gcd = {}, coefficients x = {}, y = {}.\n"
        "You can check if: {} * {} + {} * {} = {}.", digit1, digit2, gcd, x, y, x, digit1, y, digit2, gcd);

    return algorithms::to_description(numbers);
}

std::string ExampleForNDigits(int digit1, int digit2, int digit3)
{
    algorithms::euclidean euclidean;

    auto [coeff, gcd2] = euclidean(digit1, digit2, digit3);
    auto [coeff1, coeff2, coeff3] = coeff;

    std::print("\n\n\nExample2:\nStarting with {}, {}, {}, coeff = {}, {}, {}, gcd2 = {}.\n"
        "You can check if: {} * {} + {} * {} + {} * {} = {}.\n",
        digit1, digit2, digit3, coeff1, coeff2, coeff3, gcd2,
        digit1, coeff1, digit2, coeff2, digit3, coeff3, gcd2);

    return euclidean.showSteps(digit1, digit2, digit3);
}

void Menu()
{
    std::println("Choose an option:");
    std::println("1. Show the result of an Extended Euclidean Algorithm.");
    std::println("2. Show steps of completing an Extended Euclidean Algorithm.");
    std::print("Your choice: ");
}

void DisplaySteps()
{
    std::string explaination2 = ExampleForNDigits(4, 6, 8);
    std::println("Let us show how the steps look like:\n {}", explaination2);

    std::string explaination1 = ExampleForTwoDigits(546, 308);
    std::println("Let us show how the steps look like:\n {}", explaination1);
}

void ChoiceOptions()
{
    int choice;
    do {
        Menu();
        std::cin >> choice;

        switch (choice) {
        case 1:
            ExampleForNDigits(4, 6, 8);
            ExampleForTwoDigits(546, 308);
            break;
        case 2:
            DisplaySteps();
            break;
        default:
            std::println("Wrong choice. Try again.");
            break;
        }
    } while (choice != 1 && choice != 2);
}

// Test functions to see if our program calculates everything properly
bool Test(int i, int j, int k, int64_t expected_gcd)
{
    algorithms::euclidean e;
    auto [coeff, gcd] = e(i, j, k);
    auto [c1, c2, c3] = coeff;

    return (gcd == expected_gcd) && (c1 * i + c2 * j + c3 * k == expected_gcd);
}
bool Test(int i, int j) {
    algorithms::euclidean e;
    auto [coeff, gcd] = e(i, j);
    auto [c1, c2] = coeff;
    return (gcd == std::gcd(i, j)) && (c1 * i + c2 * j == std::gcd(i, j));
}

int main()
{
    ChoiceOptions();

    for (int i = 0; i < 10; i++)
    {
        for (int j = 0; j < 10; j++)
        {
            assert(Test(i, j));
        }
    }
    assert(Test(2, 4, 6, 2));
    assert(Test(546, 308, 70, 14));
}