#include <algorithm>
#include <array>
#include <print>
#include <set>
#include <valarray>
import gaussian_elimination;
import matrix;
import fraction;
import expect;


template <typename T>
auto matrix_equal(matrix<T> m1, matrix<T> m2) {
    bool is_equal = true;
    for (std::size_t i = 0; i < m1.number_of_rows(); i++) {
        for (std::size_t j = 0; j < m1.number_of_columns(); j++) {
            if (m1[i, j] != m2[i, j]) {
                is_equal = false;
                return is_equal;
            }
        }
    }
    return is_equal;
}


auto test_reduction() {
    matrix<fraction<int>> m{3, 3, {0}};
    matrix<fraction<int>> reduced_test{3, 3, {0}};
    std::array<fraction<int>, 9> m_array{
        {{7}, {2}, {4}, {5}, {5}, {3}, {7}, {4}, {9}}};
    std::array<fraction<int>, 9> reduced_array{
        {{7}, {2}, {4}, {0}, {25, 7}, {1, 7}, {0}, {0}, {123, 25}}};
    std::size_t iterator = 0;
    for (std::size_t i = 0; i < 3; i++) {
        for (std::size_t j = 0; j < 3; j++) {
            m[i, j] = m_array[iterator];
            reduced_test[i, j] = reduced_array[iterator];
            iterator++;
        }
    }
    auto reduced = utils::matrix::run(m);
    return matrix_equal(reduced, reduced_test);
}


auto test_steps() {
    matrix<fraction<int>> m{3, 3, {0}};
    std::array<fraction<int>, 9> m_array{
        {{7}, {2}, {4}, {5}, {5}, {3}, {7}, {4}, {9}}};
    std::vector<std::string> steps_test{
        "R2 - 5/7 * R1", "R3 - 1/1 * R1", "R3 - 14/25 * R2"};
    std::size_t iterator = 0;
    for (std::size_t i = 0; i < 3; i++) {
        for (std::size_t j = 0; j < 3; j++) {
            m[i, j] = m_array[iterator];
            iterator++;
        }
    }

    return testing::expect_equal(utils::matrix::show_steps(m), steps_test);
}


auto test_determinant() {
    matrix<fraction<int>> m{3, 3, {0}};
    fraction<int> determinant_test{123, 1};
    std::array<fraction<int>, 9> m_array{
        {{7}, {2}, {4}, {5}, {5}, {3}, {7}, {4}, {9}}};
    std::size_t iterator = 0;
    for (std::size_t i = 0; i < 3; i++) {
        for (std::size_t j = 0; j < 3; j++) {
            m[i, j] = m_array[iterator];
            iterator++;
        }
    }
    auto reduced = utils::matrix::run(m);
    return testing::expect_equal(utils::matrix::determinant(m).value(),
                                 determinant_test);
}


auto test_inverse() {
    matrix<fraction<int>> m{3, 3, {0}};
    matrix<fraction<int>> inverse_test{3, 3, {0}};
    std::array<fraction<int>, 9> m_array{
        {{7}, {2}, {4}, {5}, {5}, {3}, {7}, {4}, {9}}};
    std::array<fraction<int>, 9> inverse_array{{{11, 41},
                                                {-2, 123},
                                                {-14, 123},
                                                {-8, 41},
                                                {35, 123},
                                                {-1, 123},
                                                {-5, 41},
                                                {-14, 123},
                                                {25, 123}}};
    std::size_t iterator = 0;
    for (std::size_t i = 0; i < 3; i++) {
        for (std::size_t j = 0; j < 3; j++) {
            m[i, j] = m_array[iterator];
            inverse_test[i, j] = inverse_array[iterator];
            iterator++;
        }
    }
    return matrix_equal(utils::matrix::inverse(m).value(), inverse_test);
}

int main() {
    bool ok{testing::expect_equal(1, 1) &&
            testing::expect_equal(std::vector<std::int64_t>{1, 2},
                                  std::array{1, 2}) &&
            !testing::expect_equal(std::vector{1}, std::array{1, 2})};

    return std::ranges::all_of(std::array{ok,
                                          test_reduction(),
                                          test_steps(),
                                          test_determinant(),
                                          test_inverse()},
                               std::identity{})
               ? 0
               : 1;
}