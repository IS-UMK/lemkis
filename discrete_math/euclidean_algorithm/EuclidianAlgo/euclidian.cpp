#include "./euclidian.hpp"
#include <print>
#include <iostream>
#include <array>
#include <fstream>
#include <numeric>
#include <vector>
#include <cctype>
 
void saveNumbersToFile(const std::string& filename)
{
    std::ofstream file(filename); //Otwarcie pliku
    if (!file.is_open()) //Sprawdzenie czy jest otwarty
    {
        std::cerr << "Nie mozna otworzyc pliku: " << filename; //Zwracanie błędu otwarcia pliku
        return;
    }
 
    std::cout << "Podaj liczby oddzielone spacjami (wpisz 'q', aby zakonczyc):";
 
    std::vector<int> numbers; //Deklaracja wektora
    int num; //Zmienna do wstawienia elementów wektora
    while (std::cin >> num)
    {
        numbers.push_back(num); //Wstawienie do wektora kolejnych elementów
    }
 
    for (int number : numbers)
    {
        file << number << " "; //Zapisanie elementów wektora do pliku
    }
 
    file.close(); //Zamknięcie pliku
    std::cout << "Liczby zostaly zapisane do pliku: " << filename << std::endl;
}
 
// Funkcja do odczytywania liczb z pliku i umieszczania ich w std::array
template <size_t N>
std::array<int, N> readNumbersFromFile(const std::string& filename)
{
    std::ifstream file(filename);
    if (!file.is_open())
    {
        std::cerr << "Nie mozna otworzyc pliku: " << filename << "\n";
        return std::array<int, N>{}; // Zwracamy pusty std::array, jeśli nie udało się otworzyć pliku
    }
 
    std::array<int, N> numbers{};
    size_t index = 0;
    int number;
    while (file >> number && index < N)
    {
        numbers[index++] = number;
    }
 
    // Sprawdzamy, czy wczytaliśmy wystarczającą ilość liczb
    if (index < N)
    {
        std::cerr << "Plik zawiera mniej niz " << N << " liczb. Uzupelniam reszte zerami. \n";
        // Uzupełniamy resztę tablicy zerami
        for (; index < N; ++index)
        {
            numbers[index] = 0;
        }
    }
    file.close();
 
    return numbers;
};
 
//// Funkcja zliczająca liczbę cyfr w pliku
//int countDigitsInFile(const std::string& filename)
//{
//    std::ifstream file(filename);
//    if (!file.is_open())
//    {
//        std::cerr << "Nie mozna otworzyc pliku: " << filename << "\n";
//        return 0;
//    }
//
//    int digitCount = 0; // Licznik cyfr
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
//
//    return digitCount;
//}
 
int main() 
{
    std::string filename; //Utworzenie zmiennej przechowującej nazwę pliku
    std::cout << "Podaj nazwe pliku, do ktorego chcesz zapisac liczby: ";
    std::cin >> filename;
 
    saveNumbersToFile(filename); //Wywołanie funkcji 
 
    // Przykładowe użycie funkcji readNumbersFromFile
    constexpr size_t arraySize = 10; //Rozmiar tablicy
    std::array<int, arraySize> numbers = readNumbersFromFile<arraySize>(filename);
 
    // Wyświetlenie odczytanych liczb
    std::println("Liczby wczytane z pliku:\n");
    for (int num : numbers)
    {
        std::print("{}\n", num);
    }
 
	auto [coefficients, gcd] = algorithms::gcd_extended(3, 4);
	auto [x, y] = coefficients;
	std::print("na wejsciu 3, 4, gcd = {}, coefficients x = {}, y = {}\n"
		"sprawdzamy {} * 3 + {} * 4 = {}", gcd, x, y, x, y, gcd);
 
    std::array a{ 4, 6, 8 };
 
	auto [coeff, gcd2] = algorithms::gcd_extended(a);
 
	std::print("Na wejsciu {}, {}, {}, coeff = {}, {}, {}, gcd2 = {}.\n"
		"oczekujemy poprawnego gcd (sprawdzic recznie), , wspolczynniki {} = {} \n", a[0], a[1], a[2], coeff[0], coeff[1], coeff[2], gcd2, std::inner_product(coeff.begin(), coeff.end(), a.begin(), 0), gcd2);
 
	algorithms::euclidean e{};
	auto [coeff3, gcd3] = e(1, 2, 3);
	std::println("gcd3 powinno byc 1 a jest {}", gcd3);
}