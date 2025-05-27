
#include <string>
#include <vector>

constexpr size_t SMALL_SIZE = 10'000;
constexpr size_t LARGE_SIZE = 300'000'000;

void run_own_test();

std::vector<double> generate_random_vector(size_t size);

template <typename Func>
void benchmark(const std::string& name, Func func);

void test_transform(const std::vector<double>& input,
                    std::vector<double>& output);
void test_dot_product(const std::vector<double>& a,
                      const std::vector<double>& b);
