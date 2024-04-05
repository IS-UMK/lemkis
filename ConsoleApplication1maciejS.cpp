#include <iostream>
#include <vector>
#include <stdexcept>
#include <fstream>
#include <ranges>
#include <span>

namespace matrix {

    struct shape {
        std::size_t rows{}, columns{};
    };
}

namespace ranges {

    template <std::ranges::view V>
    class numeric_view : public std::ranges::view_interface<numeric_view<V>> {
    private:
        using value_type = std::ranges::range_value_t<V>;
        std::views::all_t<V> _v;

    public:
        constexpr numeric_view(V v) : _v{ v | std::views::all } {}

        constexpr auto begin() const { return _v.begin(); }
        constexpr auto begin() { return _v.begin(); }
        constexpr auto end() const { return _v.end(); }
        constexpr auto end() { return _v.end(); }

        constexpr auto operator+=(std::ranges::view auto v) {
            // Implementacja operacji dodawania na poziomie współrzędnych
            // implementacja odpowiedniej logiki
        }

        // Pozostałe operatory analogicznie
    };

    template <typename T, std::size_t extent = std::dynamic_extent>
    class matrix_view : public std::span<T, extent> {
    private:
        matrix::shape _shape{};

    public:
        constexpr matrix_view(T* data, matrix::shape s)
            : std::span<T, extent>{ data, s.rows * s.columns }, _shape{ s } {}

        constexpr auto row(std::integral auto i) {
            // Implementacja pobierania i-tego wiersza
            // implementacja odpowiedniej logiki
        }

        constexpr auto column(std::integral auto i) {
            // Implementacja pobierania i-tej kolumny
            // imprelemntacja odpowiedniej logiki
        }
    };
    // Funkcja zapisująca macierz do pliku
    template <typename T, std::size_t extent = std::dynamic_extent>
    void save(matrix_view<T, extent> m, const std::string& filename) {
        std::ofstream file(filename);
        if (!file.is_open()) {
            throw std::runtime_error("Nie można otworzyć pliku do zapisu.");
        }

        for (const auto& val : m) {
            file << val << " ";
        }
    }

} // namespace ranges

int main() {
    size_t rows, cols;
    std::cout << "Podaj liczbe wierszy i kolumn macierzy: ";
    std::cin >> rows >> cols;

    std::vector<double> data_A(rows * cols);
    std::vector<double> data_B(rows * cols);

    // Wczytaj dane pierwszej macierzy
    std::cout << "Podaj elementy pierwszej macierzy A:\n";
    for (size_t i = 0; i < rows; ++i) {
        for (size_t j = 0; j < cols; ++j) {
            double val;
            std::cout << "Element [" << i << "][" << j << "]: ";
            std::cin >> val;
            data_A[i * cols + j] = val;
        }
    }

    // Wczytaj dane drugiej macierzy
    std::cout << "Podaj elementy drugiej macierzy B:\n";
    for (size_t i = 0; i < rows; ++i) {
        for (size_t j = 0; j < cols; ++j) {
            double val;
            std::cout << "Element [" << i << "][" << j << "]: ";
            std::cin >> val;
            data_B[i * cols + j] = val;
        }
    }

    // Utwórz widoki macierzy na podstawie wczytanych danych
    matrix::shape matrix_shape{ rows, cols };
    ranges::matrix_view<double> A(data_A.data(), matrix_shape);
    ranges::matrix_view<double> B(data_B.data(), matrix_shape);

    // Przykładowe operacje na macierzach
    auto C = A + B; // Dodawanie macierzy

    // Wyświetlenie wyniku dodawania
    std::cout << "Wynik dodawania:\n";
    for (size_t i = 0; i < rows; ++i) {
        for (size_t j = 0; j < cols; ++j) {
            std::cout << C[i * cols + j] << " ";
        }
        std::cout << std::endl;
    }
    
    // Zapisuje wynik dodawania do pliku
    try {
        ranges::save(C, "result.txt");
        std::cout << "Pomyślnie zapisano wynik dodawania do pliku.\n";
    }
    catch (const std::runtime_error& e) {
        std::cerr << "Błąd: " << e.what() << std::endl;
    }

    return 0;
}
