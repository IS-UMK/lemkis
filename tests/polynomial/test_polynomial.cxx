#include <algorithm>
#include <array>
#include <cstdint>
#include <vector>

import polynomial;
import expect;

auto test_addition() -> bool {
    polynomial<int> p1{2};
    p1[0] = 3;
    p1[1] = 4;
    p1[2] = 4;
    polynomial<int> p2{1};
    p2[0] = 1;
    p2[1] = 1;
    return testing::expect_equal((p1 + p2), std::vector{4, 5, 4}) &&
           testing::expect_equal((p1 + p2).degree(), 2);
};

auto test_subtraction() -> bool {
    polynomial<int> p1{2};
    p1[0] = 3;
    p1[1] = 4;
    p1[2] = 4;
    polynomial<int> p2{1};
    p2[0] = 1;
    p2[1] = 1;
    return testing::expect_equal((p1 - p2), std::vector{2, 3, 4}) &&
           testing::expect_equal((p1 - p2).degree(), 2);
};

auto test_multiplication() -> bool {
    polynomial<int> p1{2};
    p1[0] = 3;
    p1[1] = 4;
    p1[2] = 4;
    polynomial<int> p2{1};
    p2[0] = 1;
    p2[1] = 1;
    return testing::expect_equal((p1 * p2), std::vector{3, 7, 8, 4}) &&
           testing::expect_equal((p1 * p2).degree(), 3);
};

auto test_quotient() -> bool {
    polynomial<int> p1{2};
    p1[0] = 3;
    p1[1] = 4;
    p1[2] = 4;
    polynomial<int> p2{1};
    p2[0] = 1;
    p2[1] = 1;
    return testing::expect_equal((p1 / p2), std::vector{0, 4}) &&
           testing::expect_equal((p1 / p2).degree(), 1);
};

auto test_remainder() -> bool {
    polynomial<int> p1{2};
    p1[0] = 3;
    p1[1] = 4;
    p1[2] = 4;
    polynomial<int> p2{1};
    p2[0] = 1;
    p2[1] = 1;
    return testing::expect_equal((p1 % p2), std::vector{3}) &&
           testing::expect_equal((p1 % p2).degree(), 0);
};

auto test_divide() -> bool {
    polynomial<int> p1{2};
    p1[0] = 3;
    p1[1] = 4;
    p1[2] = 4;
    polynomial<int> p2{1};
    p2[0] = 1;
    p2[1] = 1;
    auto [quo, rem] = divide(p1, p2);
    return testing::expect_equal(quo, std::vector{0, 4}) &&
           testing::expect_equal(quo.degree(), 1) &&
           testing::expect_equal(rem, std::vector{3}) &&
           testing::expect_equal(rem.degree(), 0);
};

auto test_evaluation() -> bool {
    polynomial<int> p{2};
    p[0] = 3;
    p[1] = 4;
    p[2] = 4;
    const int value = 3;
    return testing::expect_equal(p(value), 51);
};

auto test_root_rational_candidates() -> bool {
    const polynomial<int> p{2};
    p[0] = 3;
    p[1] = 4;
    p[2] = 4;
    return testing::expect_equal(utils::polynomial::root_rational_candidates(p),
                                 std::set{0, 1, 3});
};

auto test_gcd() -> bool {
    polynomial<int> p1{2};
    p1[0] = 3;
    p1[1] = 4;
    p1[2] = 4;
    polynomial<int> p2{1};
    p2[0] = 1;
    p2[1] = 1;
    return testing::expect_equal(utils::polynomial::gcd(p1, p2).first,
                                 std::vector{1, 1}) &&
           testing::expect_equal(utils::polynomial::gcd(p1, p2).second,
                                 std::vector{1});
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
