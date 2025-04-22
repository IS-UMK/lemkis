
#include <vector>
#include <string>

// Stałe do wielkości danych
constexpr size_t SMALL_SIZE = 10'000;
constexpr size_t LARGE_SIZE = 100'000'000;

// Funkcja uruchamiająca cały zestaw benchmarków
void run_own_test();

// Funkcje pomocnicze – tylko jeśli potrzebujesz osobno
std::vector<double> generate_random_vector(size_t size);

template <typename Func>
void benchmark(const std::string& name, Func func);

void test_transform(const std::vector<double>& input, std::vector<double>& output);
void test_dot_product(const std::vector<double>& a, const std::vector<double>& b);