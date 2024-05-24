#include <algorithm>
#include <cerrno>
#include <charconv>
#include <concepts>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <mdspan>
#include <print>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <system_error>
#include <tuple>
#include <utility>
#include <vector>


namespace ranges {

    /*this class will be used to modify rows columns of a matrix. It vectorizes
     * operations on ranges*/
    template <std::ranges::viewable_range R>
    class numeric_view
        : public std::ranges::subrange<std::ranges::iterator_t<R>,
                                       std::ranges::sentinel_t<R>> {

      private:
        using value_type = std::ranges::range_value_t<R>;
        using base_type = std::ranges::subrange<std::ranges::iterator_t<R>,
                                                std::ranges::sentinel_t<R>>;

      public:
        constexpr explicit numeric_view(R&& v) : base_type{v} {}
        constexpr explicit numeric_view(R& v) : base_type{v} {}


      public:
        template <std::ranges::viewable_range Range>
        requires(
            std::convertible_to<std::ranges::range_value_t<Range>, value_type>)
        constexpr auto operator+=(const Range& v) -> auto& {
            for (auto&& [a, b] : std::views::zip(*this, v)) { a += b; }
            return *this;
        }

        // Coordinate-wise subtraction
        template <std::ranges::viewable_range Range>
        requires(
            std::convertible_to<std::ranges::range_value_t<Range>, value_type>)
        constexpr auto operator-=(const Range& v) -> auto& {
            for (auto&& [a, b] : std::views::zip(*this, v)) { a -= b; }
            return *this;
        }

        // Coordinate-wise multiplication
        template <std::ranges::viewable_range Range>
        requires(
            std::convertible_to<std::ranges::range_value_t<Range>, value_type>)
        constexpr auto operator*=(const Range& v) -> auto& {
            for (auto&& [a, b] : std::views::zip(*this, v)) { a *= b; }
            return *this;
        }

        // Coordinate-wise division
        template <std::ranges::viewable_range Range>
        requires(
            std::convertible_to<std::ranges::range_value_t<Range>, value_type>)
        constexpr auto operator/=(const Range& v) -> auto& {
            for (auto&& [a, b] : std::views::zip(*this, v)) { a /= b; }
            return *this;
        }

        // Scalar addition to each coordinate
        constexpr auto operator+=(std::convertible_to<value_type> auto scalar)
            -> auto& {
            std::ranges::transform(*this, this->begin(), [scalar](auto val) {
                return val + scalar;
            });
            return *this;
        }

        // Scalar subtraction from each coordinate
        constexpr auto operator-=(std::convertible_to<value_type> auto scalar)
            -> auto& {
            std::ranges::transform(*this, this->begin(), [scalar](auto val) {
                return val - scalar;
            });
            return *this;
        }

        // Scalar multiplication to each coordinate
        constexpr auto operator*=(std::convertible_to<value_type> auto scalar)
            -> auto& {
            std::ranges::transform(*this, this->begin(), [scalar](auto val) {
                return val * scalar;
            });
            return *this;
        }

        // Scalar division from each coordinate
        constexpr auto operator/=(std::convertible_to<value_type> auto scalar)
            -> auto& {
            std::ranges::transform(*this, this->begin(), [scalar](auto val) {
                return val / scalar;
            });
            return *this;
        }
    };

}  // namespace ranges


namespace layout {
    /*given a sequence of number you can create a matrix from it filling it with
     * this sequence either row-wise or column-wise*/
    constexpr std::layout_right row{};
    constexpr std::layout_left column{};
}  // namespace layout


namespace ranges {

    constexpr std::size_t matrix_dextents{2};
    constexpr std::size_t row_extent{0};
    constexpr std::size_t column_extent{1};


    /*this is a matrix view which reinterprets a contiguous range as matrix*/
    template <typename T, typename LP>
    requires(std::same_as<LP, std::layout_right> ||
             std::same_as<LP, std::layout_left>)
    class matrix_view
        : public std::mdspan<T,
                             std::dextents<std::size_t, matrix_dextents>,
                             LP,
                             std::default_accessor<T>>,
          public std::span<T, std::dynamic_extent> {

      private:
        using mdspan_type =
            std::mdspan<T,
                        std::dextents<std::size_t, matrix_dextents>,
                        LP,
                        std::default_accessor<T>>;
        using span_type = std::span<T, std::dynamic_extent>;
        using data_handle_type = typename mdspan_type::data_handle_type;

      public:
        constexpr matrix_view(T* data,
                              std::size_t rows,
                              std::size_t columns,
                              LP /*layout*/)
            : mdspan_type{data, rows, columns},
              span_type{data, rows * columns} {};


        template <std::ranges::contiguous_range R, typename Layout>
        constexpr matrix_view(R&& data,
                              std::size_t rows,
                              std::size_t columns,
                              Layout /*layout*/)
            : mdspan_type{data.data(), rows, columns},
              span_type{data.data(), rows * columns} {};


      public:
        using mdspan_type::extent;


        constexpr auto number_of_rows() const {
            return mdspan_type::extent(row_extent);
        }


        constexpr auto number_of_columns() const {
            return mdspan_type::extent(column_extent);
        }


        /*returns the shape of this matrix, that is a pair (number of rows,
         * number of columns)*/
        [[nodiscard]] constexpr auto shape() const
            -> std::pair<std::size_t, std::size_t> {
            return {number_of_rows(), number_of_columns()};
        }

      public:
        using mdspan_type::operator[];


        // depending on layout returns ith row (layout::row) or ith column
        // (layout::column)
        constexpr auto operator[](std::size_t i) {
            auto dim{this->extent(std::same_as<LP, std::layout_right>
                                      ? column_extent
                                      : row_extent)};
            return numeric_view{std::ranges::subrange{
                this->begin() + i * dim, this->begin() + (i + 1) * dim}};
        }
    };

    template <typename T, typename LP>
    matrix_view(T*, std::size_t, std::size_t, LP) -> matrix_view<T, LP>;


    template <std::ranges::contiguous_range R, typename Layout>
    matrix_view(R&, std::size_t, std::size_t, Layout)
        -> matrix_view<std::ranges::range_value_t<R>, Layout>;


    /*return a transposition matrix view*/
    template <typename T, typename LP>
    requires(std::same_as<LP, std::layout_right> ||
             std::same_as<LP, std::layout_left>)
    constexpr auto transpose(matrix_view<T, LP> m) {
        if constexpr (std::same_as<LP, std::layout_right>) {
            return matrix_view{m.data(),
                               m.number_of_columns(),
                               m.number_of_rows(),
                               layout::column};
        } else {
            return matrix_view{m.data(),
                               m.number_of_columns(),
                               m.number_of_rows(),
                               layout::row};
        }
    }

}  // namespace ranges


/*Finds the maximum width of column when it is converted to string*/
template <typename T, typename LP>
inline auto column_widths(::ranges::matrix_view<T, LP> m)
    -> std::vector<std::size_t> {
    std::vector<std::size_t> widths{};
    for (std::size_t j = 0; j < m.number_of_columns(); ++j) {
        std::size_t width{0zU};
        for (std::size_t i = 0; i < m.number_of_rows(); ++i) {
            width = std::max(width, std::format("{}", m[i, j]).size());
        }
        widths.push_back(width);
    }
    return widths;
}

inline auto format_column(auto entry /*m[i,j]*/,
                          auto column_separator,
                          auto column_padding,
                          auto column_width /*cols_widths[j]*/,
                          bool is_last_column) -> std::string {
    if (is_last_column) {
        return (column_padding > 0)
                   ? std::format("{: ^{}}{}",
                                 std::format("{}", entry),
                                 column_width + 2 * column_padding,
                                 column_separator)
                   : std::format("{}{}", entry, column_separator);
    }
    return (column_padding > 0) ? std::format("{: ^{}}",
                                              std::format("{}", entry),
                                              column_width + 2 * column_padding)
                                : std::format("{}", entry);
}
/*converts a matrix into a string, columns are separated by column_separator,
and rows by row_separator. If column_padding == 0 no additional white spaces are
added. Otherwise matrix is prettily printed, that is columns are aligned and
padded left and right using std::string(column_padding, ' '). In particular,
when you save to file use column_padding = 0*/
template <typename T, typename LP>
auto to_string(::ranges::matrix_view<T, LP> m,
               char column_separator,
               char row_separator,
               std::size_t column_padding = 1) {
    std::string out{(column_padding > 0) ? "\n" : ""};
    const auto cols_widths{column_widths(m)};
    for (std::size_t i = 0; i < m.number_of_rows(); ++i) {
        for (std::size_t j = 0; j < m.number_of_columns(); ++j) {
            out += format_column(m[i, j],
                                 column_separator,
                                 column_padding,
                                 cols_widths[j],
                                 (j + 1 < m.number_of_columns()));
        }
        if (i + 1 < m.number_of_rows()) { out += row_separator; }
    }
    return out;
}

/*formatter for matrix of the form
 * {:<column_separator><row_separator><column_padding_value>}*/
template <typename T, typename LP>
struct std::formatter<::ranges::matrix_view<T, LP>, char> {
    char column_separator{','};
    char row_separator{'\n'};
    std::size_t column_padding{1zU};
    template <class ParseContext>
    constexpr auto parse(ParseContext& ctx) -> ParseContext::iterator {
        auto pos = ctx.begin();
        std::size_t i{0};
        while (pos != ctx.end() && *pos != '}') {
            if (i == 0) { column_separator = *pos; }
            if (i == 1) { row_separator = *pos; }
            if (i == 2) {
                column_padding =
                    std::min(static_cast<std::size_t>(*pos - '0'), 4zU);
            }
            ++i;
            ++pos;
        }
        return pos;
    }

    template <class FmtContext>
    auto format(::ranges::matrix_view<T, LP> m, FmtContext& ctx) const
        -> FmtContext::iterator {
        return std::format_to(
            ctx.out(), "{}", ::to_string(m, column_separator, row_separator));
    }
};


namespace utils {

    /*loads a content of file to std::string, in case of error return empty
     * string*/
    template <typename C>
    auto load(std::filesystem::path file) -> std::basic_string<C> {
        std::basic_string<C> data{};
        std::error_code ec{};
        const auto sz = std::filesystem::file_size(file, ec);
        if (ec) {
            std::print("file size error: {}\n", ec.message());
            return data;
        }
        std::basic_ifstream<C> f(file, std::ios::in | std::ios::binary);
        data.resize(sz);
        f.read(data.data(), sz);
        return data;
    }

    /*converts a string to type T and returns it*/
    template <typename T>
    inline auto from_chars(std::string_view value) -> T {
        T out{};
        std::from_chars(value.data(), value.data() + value.size(), out);
        return out;
    }

}  // namespace utils


namespace matrix {

    /*saves a matrix m to a file file, using column_spearator and
     * row_separator*/
    template <char ColumnSeparator = ',',
              char RowSeparator = '\n',
              typename T,
              typename LP>
    inline auto save(ranges::matrix_view<T, LP> m,
                     const std::filesystem::path& file) -> void {
        std::filesystem::create_directories(file.parent_path());
        if (std::FILE * stream{std::fopen(file.c_str(), "w")}) {
            std::print(
                stream, "{}", ::to_string(m, ColumnSeparator, RowSeparator, 0));
            std::fclose(stream);
        } else {
            std::print("failed to create file {}, reason = {}\n",
                       file.string(),
                       std::strerror(errno));
        }
    }


    template <typename T, char ColumnSeparator = ',', char RowSeparator = ';'>
    inline auto to_shape(std::string_view content)
        -> std::pair<std::size_t, std::size_t> {
        const auto row_seps{static_cast<std::size_t>(
            std::ranges::count(content, RowSeparator))};
        const auto col_seps{static_cast<std::size_t>(
            std::ranges::count(content, ColumnSeparator))};
        const std::size_t rows{row_seps + 1};
        const std::size_t cols{col_seps / rows + 1};
        return {rows, cols};
    }


    /*given a string representing a matrix which separators are column_separator
    and row_separator finds its shape and returns a triple: a vector with values
    converted to the type T, number of rows, number of columns.*/
    template <typename T, char ColumnSeparator = ',', char RowSeparator = ';'>
    inline auto to_values_and_shape(std::string_view content)
        -> std::tuple<std::vector<T>, std::size_t, std::size_t> {
        if (content.empty()) { return {std::vector<T>{}, 0zU, 0zU}; }
        const auto [rows, cols] =
            to_shape<T, ColumnSeparator, RowSeparator>(content);
        std::vector<T> values{};
        for (auto&& row :
             content | std::views::split(std::views::single(RowSeparator))) {
            std::ranges::copy(
                row | std::views::split(std::views::single(ColumnSeparator)) |
                    std::views::transform([](auto&& value) {
                        return utils::from_chars<T>(std::string_view{value});
                    }),
                std::back_inserter(values));
        }
        return {std::move(values), rows, cols};
    }


    /*loads a matrix from a file assuming that this matrix uses separators
    column_separator and row_separator. Moreover, assumes that the entries of
    this matrix can be converted to type T. Returns a pair: a vector containing
    values of matrix and the corresponding matrix_view*/
    template <typename T, char ColumnSeparator = ',', char RowSeparator = ';'>
    [[nodiscard]] inline auto load(std::filesystem::path file)
        -> std::pair<std::vector<T>,
                     ranges::matrix_view<T, std::layout_right>> {
        std::string const content{utils::load<char>(std::move(file))};
        auto [matrix_values, rows, cols] =
            to_values_and_shape<T, ColumnSeparator, RowSeparator>(content);

        ::ranges::matrix_view m{matrix_values, rows, cols, layout::row};
        return {std::move(matrix_values), std::move(m)};
    }


}  // namespace matrix
