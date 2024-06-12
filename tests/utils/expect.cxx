module;

#include <concepts>
#include <format>
#include <print>
#include <ranges>
#include <valarray>

export module expect;


export namespace testing {


    template <typename T, typename S>
    requires(std::equality_comparable_with<T, S>) and
                (not std::ranges::range<T>) and (not std::ranges::range<S>)
    inline auto expect_equal(T t, S s) -> bool {
        return (t == s);
    }


    template <std::ranges::range T, std::ranges::range S>
    inline auto expect_equal(T t, S s) -> bool {
        if (std::distance(std::ranges::begin(t), std::ranges::end(t)) !=
            std::distance(std::ranges::begin(s), std::ranges::end(s))) {
            return false;
        }
        auto [t_mismatch, s_mismatch] =
            std::ranges::mismatch(t,
                                  s,
                                  expect_equal<std::ranges::range_value_t<T>,
                                               std::ranges::range_value_t<S>>);

        return (t_mismatch == std::ranges::end(t) &&
                s_mismatch == std::ranges::end(s));
    }


    template <std::ranges::range T, typename S>
    inline auto expect_equal(T t, std::slice_array<S> s) -> bool {
        return expect_equal(t, std::valarray(s));
    }


    template <typename T, std::ranges::range S>
    inline auto expect_equal(std::slice_array<T> t, S s) -> bool {
        return expect_equal(std::valarray(t), s);
    }
}  // namespace testing