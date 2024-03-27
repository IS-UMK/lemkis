#include <iostream>
#include <vector>
#include <stdexcept>
#include <fstream>

class Macierz {
private:
    std::vector<std::vector<double>> dane;
    size_t rows;
    size_t cols;

public:
    // Konstruktor
    Macierz(size_t rows, size_t cols) : rows(rows), cols(cols) {
        dane.resize(rows, std::vector<double>(cols, 0.0));
    }

    // Metoda zapisu danych do macierzy
    void zapisz(size_t row, size_t col, double val) {
        if (row >= rows || col >= cols)
            throw std::out_of_range("Nieprawidłowe indeksy");
        dane[row][col] = val;
    }

    // Metoda ładowania danych do macierzy
    double pobierz(size_t row, size_t col) const {
        if (row >= rows || col >= cols)
            throw std::out_of_range("Nieprawidłowe indeksy");
        return dane[row][col];
    }

    // Metoda pobierająca wiersz
    std::vector<double> pobierzWiersz(size_t row) const {
        if (row >= rows)
            throw std::out_of_range("Nieprawidłowy wiersz");
        return dane[row];
    }

    // Metoda pobierająca kolumnę
    std::vector<double> pobierzKolumne(size_t col) const {
        if (col >= cols)
            throw std::out_of_range("Nieprawidłowa kolumna");
        std::vector<double> kolumna;
        for (size_t i = 0; i < rows; ++i)
            kolumna.push_back(dane[i][col]);
        return kolumna;
    }

    // Przeciążony operator +
    Macierz operator+(const Macierz& other) const {
        if (rows != other.rows || cols != other.cols)
            throw std::invalid_argument("Macierze muszą być tego samego rozmiaru");
        Macierz result(rows, cols);
        for (size_t i = 0; i < rows; ++i) {
            for (size_t j = 0; j < cols; ++j) {
                result.dane[i][j] = dane[i][j] + other.dane[i][j];
            }
        }
        return result;
    }

    // Przeciążony operator -
    Macierz operator-(const Macierz& other) const {
        if (rows != other.rows || cols != other.cols)
            throw std::invalid_argument("Macierze muszą być tego samego rozmiaru");
        Macierz result(rows, cols);
        for (size_t i = 0; i < rows; ++i) {
            for (size_t j = 0; j < cols; ++j) {
                result.dane[i][j] = dane[i][j] - other.dane[i][j];
            }
        }
        return result;
    }

    // Przeciążony operator /
    Macierz operator/(const Macierz& other) const {
        if (rows != other.rows || cols != other.cols)
            throw std::invalid_argument("Macierze muszą być tego samego rozmiaru");
        Macierz result(rows, cols);
        for (size_t i = 0; i < rows; ++i) {
            for (size_t j = 0; j < cols; ++j) {
                if (other.dane[i][j] == 0.0)
                    throw std::invalid_argument("Dzielenie przez zero");
                result.dane[i][j] = dane[i][j] / other.dane[i][j];
            }
        }
        return result;
    }

    // Funkcja do dodawania macierzy
    static Macierz dodaj(const Macierz& A, const Macierz& B) {
        return A + B;
    }

    // Funkcja do odejmowania macierzy
    static Macierz odejmij(const Macierz& A, const Macierz& B) {
        return A - B;
    }

    // Funkcja do dzielenia macierzy
    static Macierz podziel(const Macierz& A, const Macierz& B) {
        return A / B;
    }

    // Metoda obliczająca iloczyn skalarny dwóch macierzy
    static double iloczynSkalarny(const Macierz& A, const Macierz& B) {
        if (A.rows != B.rows || A.cols != B.cols)
            throw std::invalid_argument("Macierze muszą być tego samego rozmiaru");
        double wynik = 0.0;
        for (size_t i = 0; i < A.rows; ++i) {
            for (size_t j = 0; j < A.cols; ++j) {
                wynik += A.dane[i][j] * B.dane[i][j];
            }
        }
        return wynik;
    }

    // Funkcja zapisująca macierz do pliku
    void zapiszDoPliku(const std::string& nazwaPliku) const {
        std::ofstream plik(nazwaPliku);
        if (!plik.is_open()) {
            throw std::runtime_error("Nie można otworzyć pliku do zapisu");
        }

        for (size_t i = 0; i < rows; ++i) {
            for (size_t j = 0; j < cols; ++j) {
                plik << dane[i][j] << " ";
            }
            plik << std::endl;
        }

        plik.close();
    }
};

int main() {
    size_t rows, cols;
    std::cout << "Podaj liczbe wierszy i kolumn macierzy: ";
    std::cin >> rows >> cols;

    Macierz A(rows, cols);
    Macierz B(rows, cols);

    std::cout << "Podaj elementy pierwszej macierzy A:\n";
    for (size_t i = 0; i < rows; ++i) {
        for (size_t j = 0; j < cols; ++j) {
            double val;
            std::cout << "Element [" << i << "][" << j << "]: ";
            std::cin >> val;
            A.zapisz(i, j, val);
        }
    }

    std::cout << "Podaj elementy drugiej macierzy B:\n";
    for (size_t i = 0; i < rows; ++i) {
        for (size_t j = 0; j < cols; ++j) {
            double val;
            std::cout << "Element [" << i << "][" << j << "]: ";
            std::cin >> val;
            B.zapisz(i, j, val);
        }
    }

    std::cout << "Wybierz operację:\n";
    std::cout << "1. Dodawanie macierzy\n";
    std::cout << "2. Odejmowanie macierzy\n";
    std::cout << "3. Iloczyn skalarny macierzy\n";
    std::cout << "4. Dzielenie macierzy\n";
    int choice;
    std::cin >> choice;

    std::string nazwaPliku = "wynik.txt";

    switch (choice) {
    case 1:
    {
        Macierz C = Macierz::dodaj(A, B);
        std::cout << "Wynik dodawania:\n";
        C.zapiszDoPliku(nazwaPliku);
        for (size_t i = 0; i < rows; ++i) {
            for (size_t j = 0; j < cols; ++j) {
                std::cout << C.pobierz(i, j) << " ";
            }
            std::cout << std::endl;
        }
    }
    break;
    case 2:
    {
        Macierz C = Macierz::odejmij(A, B);
        std::cout << "Wynik odejmowania:\n";
        C.zapiszDoPliku(nazwaPliku);
        for (size_t i = 0; i < rows; ++i) {
            for (size_t j = 0; j < cols; ++j) {
                std::cout << C.pobierz(i, j) << " ";
            }
            std::cout << std::endl;
        }
    }
    break;
    case 3:
    {
        try {
            double iloczyn = Macierz::iloczynSkalarny(A, B);
            std::ofstream plik(nazwaPliku);
            plik << "Iloczyn skalarny macierzy: " << iloczyn << std::endl;
            plik.close();
            std::cout << "Iloczyn skalarny macierzy: " << iloczyn << std::endl;
        }
        catch (const std::invalid_argument& e) {
            std::cerr << "Błąd: " << e.what() << std::endl;
        }
    }
    break;
    case 4:
    {
        try {
            Macierz C = Macierz::podziel(A, B);
            std::cout << "Wynik dzielenia:\n";
            C.zapiszDoPliku(nazwaPliku);
            for (size_t i = 0; i < rows; ++i) {
                for (size_t j = 0; j < cols; ++j) {
                    std::cout << C.pobierz(i, j) << " ";
                }
                std::cout << std::endl;
            }
        }
        catch (const std::invalid_argument& e) {
            std::cerr << "Błąd: " << e.what() << std::endl;
        }
    }
    break;
    default:
        std::cout << "Nieprawidłowy wybór.\n";
    }

    return 0;
}