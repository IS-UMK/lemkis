#include "./euclidian.hpp"
#include <print>
#include <iostream>
#include <array>
#include <fstream>
#include <numeric>
#include <vector>
#include <cctype>

//void saveNumbersToFile(const std::string& filename)
//{
// //opening the file
// std::ofstream file(filename); 
// //checking if the file is open
//    if (!file.is_open()) 
//    {
//        //returning error while opening the file
//        std::cerr << "Cannot open the file: " << filename; 
//        return;
//    }
//
//    std::cout << "insert numbers separated with spacebar (enter 'q', in order to finish):";
//
//    //declaration of the vector
//    std::vector<int> numbers;
//    //variable for inserting vector elements
//    int num; 
//    while (std::cin >> num)
//    {
//        //inserting subsequent elements into the vector
//        numbers.push_back(num); 
//    }
//
//    for (int number : numbers)
//    {
//        //saving vector elements to file
//        file << number << " "; 
//    }
//
//    //closing the file
//    file.close(); 
//    std::cout << "Numbers were saved to the file: " << filename << std::endl;
//}

////a function meant to read numbers from a file and placing (inserting) them in std::array
//template <size_t N>
//std::array<int, N> readNumbersFromFile(const std::string& filename)
//{
//    std::ifstream file(filename);
//    if (!file.is_open())
//    {
//        std::cerr << "Failed to open the file: " << filename << "\n";
//        //we return an empty std::array, if the file failed to open
//        return std::array<int, N>{}; 
//    }
//
//    std::array<int, N> numbers{};
//    size_t index = 0;
//    int number;
//    while (file >> number && index < N)
//    {
//        numbers[index++] = number;
//    }
//
//    //checking whether we loaded enough numbers 
//    if (index < N)
//    {
//        std::cerr << "File has less than " << N << " numbers. Filling the rest with 0's. \n";
//        //filling rest of an array with 0's
//        for (; index < N; ++index)
//        {
//            numbers[index] = 0;
//        }
//    }
//    file.close();
//
//    return numbers;
//};

//// A function that counts the number of digits in a file
//int countDigitsInFile(const std::string& filename)
//{
//    std::ifstream file(filename);
//    if (!file.is_open())
//    {
//        std::cerr << "Failed to open the file: " << filename << "\n";
//        return 0;
//    }
//
//    //digit count
//    int digitCount = 0;
//
//    char character;
//    while (file.get(character))
//    {
//        if (std::isdigit(character))
//        {
//            ++digitCount;
//        }
//    }
//
//    file.close();
//    return digitCount;
//}

void menu() {
    std::println("Choose an option:");
    std::println("1. Show the result of an Extended Euclidean Algorithm.");
    std::println("2. Show steps of completing an Extended Euclidean Algorithm.");
    std::print("Your choice: ");
}

void displayResult() {
    auto [coefficients, gcd] = algorithms::gcd_extended(3, 4);
    auto [x, y] = coefficients;
    std::println("Starting with 3, 4, GCD = {}, coefficients x = {}, y = {}", gcd, x, y);
}

void displaySteps() {
    std::array<int, 2> numbers;
    std::println("Give out two numbers separated by spacebar: ");
    std::cin >> numbers[0] >> numbers[1];

    algorithms::euclidean e{};
    e.showSteps(numbers[0], numbers[1]);
}

int main()
{
    // creating a variable that stores name of the file
    //std::string filename;
    //std::cout << "Name the file to which you want to save the numbers: ";
    //std::cin >> filename;

    //calling out the function
    //saveNumbersToFile(filename); 

    //// example use of the readNumbersFromFile function
    //// array size
    //constexpr size_t arraySize = 10; 
    //std::array<int, arraySize> numbers = readNumbersFromFile<arraySize>(filename);

    //// Display of read numbers
    //std::println("Numbers read from the file:\n");
    //for (int num : numbers)
    //{
    //    std::print("{}\n", num);
    //}

    int choice;
    do {
        menu();
        std::cin >> choice;

        switch (choice) {
        case 1:
            displayResult();
            break;
        case 2:
            displaySteps();
            break;
        default:
            std::println("Wrong choice. Try again.");
            break;
        }
    } while (choice != 1 && choice != 2);


    auto [coefficients, gcd] = algorithms::gcd_extended(3, 4);
    auto [x, y] = coefficients;
    std::println("starting 3, 4, gcd = {}, coefficients x = {}, y = {}\n"
        "checking if {} * 3 + {} * 4 = {}", gcd, x, y, x, y, gcd);

    std::array a{ 4, 6, 8 };

    auto [coeff, gcd2] = algorithms::gcd_extended(a);

    std::print("starting with {}, {}, {}, coeff = {}, {}, {}, gcd2 = {}.\n"
        "we expect correct gcd (sprawdzic recznie), , coefficients {} = {} \n", a[0], a[1], a[2], coeff[0], coeff[1], coeff[2], gcd2, std::inner_product(coeff.begin(), coeff.end(), a.begin(), 0), gcd2);

    algorithms::euclidean e{};
    auto [coeff3, gcd3] = e(1, 2, 3);
    std::println("gcd3 should be 1 but is {}", gcd3);
}