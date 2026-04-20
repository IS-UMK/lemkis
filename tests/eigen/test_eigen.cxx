#include <algorithm>
#include <array>
#include <cstdint>
#include <vector>

import eigen;
import matrix;
import expect;

template <typename T>
inline auto abs_m(matrix<T> m) -> matrix<T> {
    for (std::size_t i = 0; i < m.number_of_rows(); i++) {
        for (std::size_t j = 0; j < m.number_of_columns(); j++) {
            m[i, j] = abs(m[i, j]);
        }
    }
    return m;
}

template <typename T>
inline auto check_tolerance(matrix<T> v1, matrix<T> v2) -> bool {
    assert(v1.number_of_columns() == 1 && v1.shape() == v2.shape());
    const double tolerance{1e-3};
    matrix<T> m_diff{abs_m(v1) - abs_m(v2)};
    for (std::size_t i = 0; i < m_diff.number_of_rows(); i++) {
        if (abs(m_diff[i, 0]) > tolerance) { return false; }
    }
    return true;
}

auto test_power_method() -> bool {
    matrix<double> m{3, 3, 0};
        m[0, 0] = 4;
        m[0, 1] = 2;
        m[0, 2] = 5;
        m[1, 0] = 2;
        m[1, 1] = 2;
        m[1, 2] = 4;
        m[2, 0] = 3;
        m[2, 1] = 1;
        m[2, 2] = 2;
    matrix<double> v_expected{3, 1, 0};
        v_expected[0, 0] = 0.742;
        v_expected[1, 0] = 0.511;
        v_expected[2, 0] = 0.434;
    return check_tolerance(eigen::power_method(m), v_expected);
};


auto test_inverse_power_method() -> bool {
    matrix<double> m{3, 3, 0};
        m[0, 0] = 4;
        m[0, 1] = 2;
        m[0, 2] = 5;
        m[1, 0] = 2;
        m[1, 1] = 2;
        m[1, 2] = 4;
        m[2, 0] = 3;
        m[2, 1] = 1;
        m[2, 2] = 2;
    matrix<double> v_expected{3, 1, 0};
        v_expected[0, 0] = 0.742;
        v_expected[1, 0] = 0.511;
        v_expected[2, 0] = 0.434;
    return check_tolerance(eigen::inverse_power_method(m), v_expected);
};

auto test_rayleigh() -> bool {
    matrix<double> m{3, 3, 0};
        m[0, 0] = 4;
        m[0, 1] = 2;
        m[0, 2] = 5;
        m[1, 0] = 2;
        m[1, 1] = 2;
        m[1, 2] = 4;
        m[2, 0] = 3;
        m[2, 1] = 1;
        m[2, 2] = 2;
    matrix<double> v_expected{3, 1, 0};
        v_expected[0, 0] = 0.742;
        v_expected[1, 0] = 0.511;
        v_expected[2, 0] = 0.434;
    return check_tolerance(eigen::rayleigh(m), v_expected);
};

auto test_qr() -> bool {
    matrix<double> m{3, 3, 0};
        m[0, 0] = 4;
        m[0, 1] = 2;
        m[0, 2] = 5;
        m[1, 0] = 2;
        m[1, 1] = 2;
        m[1, 2] = 4;
        m[2, 0] = 3;
        m[2, 1] = 1;
        m[2, 2] = 2;
    matrix<double> v_expected{3, 1, 0};
        v_expected[0, 0] = 0.742;
        v_expected[1, 0] = 0.480;
        v_expected[2, 0] = 0.267;
    return check_tolerance(eigen::qr(m), v_expected);
};

auto test_jacobi() -> bool {
    matrix<double> m{3, 3, 0};
        m[0, 0] = 4;
        m[0, 1] = 2;
        m[0, 2] = 5;
        m[1, 0] = 2;
        m[1, 1] = 2;
        m[1, 2] = 4;
        m[2, 0] = 3;
        m[2, 1] = 1;
        m[2, 2] = 2;
    matrix<double> v_expected{3, 1, 0};
        v_expected[0, 0] = 8.094;
        v_expected[1, 0] = 0.767;
        v_expected[2, 0] = -0.862;
    return check_tolerance(eigen::jacobi(m), v_expected);
};

int main(int argc, char const *argv[]) {


    bool ok{testing::expect_equal(1, 1) &&
            testing::expect_equal(std::vector<std::int64_t>{1, 2},
                                  std::array{1, 2}) &&
            !testing::expect_equal(std::vector{1}, std::array{1, 2})};
    return std::ranges::all_of(std::array{ok,
                                          test_power_method(),
                                          test_inverse_power_method(),
                                          test_rayleigh(),
                                          test_qr(),
                                          test_jacobi()},
                               std::identity{})
               ? 0
               : 1;
}
