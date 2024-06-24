#include <algorithm>
#include <array>
#include <cstdint>
#include <vector>

import polynomial;
import expect;

auto test_addition() -> bool {
    const polynomial::polynomial<int> p1{{3, 4, 4}, 2};
    const polynomial::polynomial<int> p2{{1, 1}, 1};
    return testing::expect_equal((p1 + p2).coefficients,
                                 std::vector{4, 5, 4}) &&
           testing::expect_equal((p1 + p2).degree, 2);
};

auto test_subtraction() -> bool {
    const polynomial::polynomial<int> p1{{3, 4, 4}, 2};
    const polynomial::polynomial<int> p2{{1, 1}, 1};
    return testing::expect_equal((p1 - p2).coefficients,
                                 std::vector{2, 3, 4}) &&
           testing::expect_equal((p1 - p2).degree, 2);
};

auto test_multiplication() -> bool {
    const polynomial::polynomial<int> p1{{3, 4, 4}, 2};
    const polynomial::polynomial<int> p2{{1, 1}, 1};
    return testing::expect_equal((p1 * p2).coefficients,
                                 std::vector{3, 7, 8, 4}) &&
           testing::expect_equal((p1 * p2).degree, 3);
};

auto test_quotient() -> bool {
    const polynomial::polynomial<int> p1{{3, 4, 4}, 2};
    const polynomial::polynomial<int> p2{{1, 1}, 1};
    return testing::expect_equal((p1 / p2).coefficients, std::vector{0, 4}) &&
           testing::expect_equal((p1 / p2).degree, 1);
};

auto test_remainder() -> bool {
    const polynomial::polynomial<int> p1{{3, 4, 4}, 2};
    const polynomial::polynomial<int> p2{{1, 1}, 1};
    return testing::expect_equal((p1 % p2).coefficients, std::vector{3}) &&
           testing::expect_equal((p1 % p2).degree, 0);
};

auto test_divide() -> bool {
    const polynomial::polynomial<int> p1{{3, 4, 4}, 2};
    const polynomial::polynomial<int> p2{{1, 1}, 1};
    auto [quo, rem] = divide(p1, p2);
    return testing::expect_equal(quo.coefficients, std::vector{0, 4}) &&
           testing::expect_equal(quo.degree, 1) &&
           testing::expect_equal(rem.coefficients, std::vector{3}) &&
           testing::expect_equal(rem.degree, 0);
};

auto test_evaluation() -> bool {
    polynomial::polynomial<int> p{{3, 4, 4}, 2};
    const int value = 3;
    return testing::expect_equal(p(value), 51);
};

auto test_root_rational_candidates() -> bool {
    const polynomial::polynomial<int> p{{3, 4, 4}, 2};
    return testing::expect_equal(root_rational_candidates(p),
                                 std::set{0, 1, 3});
};

auto test_gcd() -> bool {
    const polynomial::polynomial<int> p1{{3, 4, 4}, 2};
    const polynomial::polynomial<int> p2{{1, 1}, 1};
    return testing::expect_equal(gcd(p1, p2).first.coefficients,
                                 std::vector{1, 1}) &&
           testing::expect_equal(gcd(p1, p2).second, std::vector{1});
};

int main(int argc, char const *argv[]) {
    bool ok{testing::expect_equal(1, 1) &&
            testing::expect_equal(std::vector<std::int64_t>{1, 2},
                                  std::array{1, 2}) &&
            !testing::expect_equal(std::vector{1}, std::array{1, 2})};
    return std::ranges::all_of(std::array{ok,
                                          test_addition(),
                                          test_subtraction(),
                                          test_multiplication(),
                                          test_quotient(),
                                          test_remainder(),
                                          test_divide(),
                                          test_evaluation(),
                                          test_root_rational_candidates(),
                                          test_gcd()},
                               std::identity{})
               ? 0
               : 1;
}