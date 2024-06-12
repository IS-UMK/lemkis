#include <print>
#include <vector>
#include <ranges>
#include <cassert>


import matrix;



namespace std {

    template <typename R>
    requires(std::ranges::range<R>) && (true)
    struct formatter<R> {

      public:
        using range_value_type =
            std::remove_cvref_t<std::ranges::range_value_t<R>>;


      public:
        constexpr auto parse(std::format_parse_context &ctx) {
            return ctx.begin();
            ;
        }
        auto format(R r, std::format_context &ctx) const ->
            typename std::remove_cvref_t<decltype(ctx)>::iterator {
            if constexpr (std::same_as<range_value_type, char> ||
                          std::same_as<range_value_type, char8_t>) {
                std::string out{};
                for (auto c : r) { out += c; }
                // std::ranges::copy(r, std::back_inserter(out));
                return std::format_to(ctx.out(), "{}", out);
            } else /*(std::ranges::range<range_value_type>)*/ {
                std::string out{};
                out += '[';
                std::ranges::copy(
                    r | std::views::transform([](auto &&subr) {
                        return std::format("{}", subr);
                    }) | std::views::join_with(std::string_view{", "}),
                    std::back_inserter(out));
                out += ']';
                return std::format_to(ctx.out(), "{}", out);
            }
        }
    };

}  // namespace std

int main() {
   matrix<int> m{3, 3, 0};
   m[1,2] = 3;
   m[0,1] = 1;
   std::print("matrix m = {}\n", m);
   std::print("matrix m transposed = {}\n", utils::matrix::transpose(m));
   assert(std::ranges::equal(std::valarray(m.row(0)), std::valarray{0, 1, 0}));
   for (std::size_t i = 0; i < m.number_of_rows(); ++i) {
      std::println("matrix m row {} = {}", i, std::valarray(m.row(i)));
   }
   for (std::size_t i = 0; i < m.number_of_columns(); ++i) {
      std::println("matrix m col {} = {}", i, std::valarray(m.column(i)));
   }
   std::print("identity matrix = {}\n", utils::matrix::identity<int>(3));
   std::print("diagonal matrix = {}\n", utils::matrix::eye<int>({2,3,4}));
   std::print("diagonal matrix = {}\n", utils::matrix::eye(22,33,44));
   return 0;
}
