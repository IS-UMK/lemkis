#include <cassert>
#include <print>
#include <ranges>
#include <vector>


import matrix;
import expect;


int main() {
    /*here you can play with your code :) this file is not checked by any of
     * clang-tools*/
    matrix<int> m{3, 3, 0};
    m[1, 2] = 3;
    m[0, 1] = 1;
    std::print("matrix m = {}\n", m);
    std::print("matrix m transposed = {}\n", utils::matrix::transpose(m));
    for (std::size_t i = 0; i < m.number_of_rows(); ++i) {
        std::println("matrix m row {} = {}", i, std::valarray(m.row(i)));
    }
    for (std::size_t i = 0; i < m.number_of_columns(); ++i) {
        std::println("matrix m col {} = {}", i, std::valarray(m.column(i)));
    }
    std::print("identity matrix = {}\n", utils::matrix::identity<int>(3));
    std::print("diagonal matrix = {}\n", utils::matrix::eye<int>({2, 3, 4}));
    std::print("diagonal matrix = {}\n", utils::matrix::eye(22, 33, 44));


    bool ok{testing::expect_equal(1, 1) &&
            testing::expect_equal(std::vector<std::int64_t>{1, 2},
                                  std::array{1, 2})};


    matrix m1{3, 4, 0}, m2{3, 4, 1};
    m1[0, 0] = 1;
    m1[1, 2] = 3;
    std::print("matrix m1  = {}\n and m1 as range = {}\n",
               m1,
               static_cast<std::valarray<int> &>(m1));
    std::print("{}\n+\n{}\n=\n{}\n\n", m1, m2, m1 + m2);
    std::print("{}\n+\n{}\n=\n{}\n\n", m1, 1, m1 + 1);

    return 0;
}
