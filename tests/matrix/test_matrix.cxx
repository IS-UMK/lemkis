import matrix;
import expect;

#include <algorithm>
#include <array>
#include <cstdint>
#include <vector>


auto test_shape() -> bool {
    const matrix m{3, 4, 0};
    return testing::expect_equal(m.number_of_rows(), 3) &&
           testing::expect_equal(m.number_of_columns(), 4) &&
           testing::expect_equal(m.shape(), std::pair{3, 4});
};


auto test_access_operator() -> bool {
    matrix m{3, 4, 0};
    m[0, 0] = 1;
    m[1, 2] = 3;
    return testing::expect_equal(
               m, std::array{1, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0}) &&
           testing::expect_equal(m.row(0), std::array{1, 0, 0, 0}) &&
           testing::expect_equal(m.diagonal(), std::array{1, 0, 0}) &&
           testing::expect_equal(m.column(2), std::array{0, 3, 0}) &&
           testing::expect_equal(m[1, 4, 2], std::array{0, 0, 0, 0});
};


auto test_numeric_operators() -> bool {
    matrix m{3, 4, 0};
    m += 1;
    bool is_ok{testing::expect_equal(m, matrix{3, 4, 1})};
    matrix m1{3, 4, 1};
    m1[0, 0] = 2;
    m += m1;
    matrix m_exp{3, 4, 2};
    m_exp[0, 0] = 3;
    return is_ok && testing::expect_equal(m, m_exp);
};


auto test_transpose() -> bool {
    matrix m{3, 4, 0};
    m[0, 0] = 1;
    m[1, 2] = 3;
    matrix m_transposed{4, 3, 0};
    m_transposed[0, 0] = 1;
    m_transposed[2, 1] = 3;

    return testing::expect_equal(utils::matrix::transpose(m), m_transposed);
};


auto test_identity() -> bool {


    return testing::expect_equal(utils::matrix::identity<int>(3),
                                 std::array{1, 0, 0, 0, 1, 0, 0, 0, 1});
};


auto test_eye() -> bool {
    return testing::expect_equal(utils::matrix::eye(1, 2, 3),
                                 std::array{1, 0, 0, 0, 2, 0, 0, 0, 3});
};


int main(int argc, char const *argv[]) {


    bool ok{testing::expect_equal(1, 1) &&
            testing::expect_equal(std::vector<std::int64_t>{1, 2},
                                  std::array{1, 2}) &&
            !testing::expect_equal(std::vector{1}, std::array{1, 2})};
    return std::ranges::all_of(std::array{ok,
                                          test_shape(),
                                          test_access_operator(),
                                          test_numeric_operators(),
                                          test_transpose(),
                                          test_identity(),
                                          test_eye()},
                               std::identity{})
               ? 0
               : 1;
}