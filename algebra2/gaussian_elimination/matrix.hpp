#include <algorithm>
#include <cstdio>
#include <charconv>
#include <expected>
#include <filesystem>
#include <format>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <mdspan>
#include <print>
#include <ranges>
#include <sstream>
#include <string_view>


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
        constexpr numeric_view(R&& v) : base_type{v} {}
        constexpr numeric_view(R& v) : base_type{v} {}


      public:
        template <std::ranges::viewable_range _R>
        requires(
            std::convertible_to<std::ranges::range_value_t<_R>, value_type>)
        constexpr auto& operator+=(const _R& v) {
            for (auto&& [a, b] : std::views::zip(*this, v)) { a += b; }
            return *this;
        }

        // Coordinate-wise subtraction
        template <std::ranges::viewable_range _R>
        requires(
            std::convertible_to<std::ranges::range_value_t<_R>, value_type>)
        constexpr auto& operator-=(const _R& v) {
            for (auto&& [a, b] : std::views::zip(*this, v)) { a -= b; }
            return *this;
        }

        // Coordinate-wise multiplication
        template <std::ranges::viewable_range _R>
        requires(
            std::convertible_to<std::ranges::range_value_t<_R>, value_type>)
        constexpr auto& operator*=(const _R& v) {
            for (auto&& [a, b] : std::views::zip(*this, v)) { a *= b; }
            return *this;
        }

        // Coordinate-wise division
        template <std::ranges::viewable_range _R>
        requires(
            std::convertible_to<std::ranges::range_value_t<_R>, value_type>)
        constexpr auto& operator/=(const _R& v) {
            for (auto&& [a, b] : std::views::zip(*this, v)) { a /= b; }
            return *this;
        }

        // Scalar addition to each coordinate
        constexpr auto& operator+=(
            std::convertible_to<value_type> auto scalar) {
            std::ranges::transform(*this, this->begin(), [scalar](auto val) {
                return val + scalar;
            });
            return *this;
        }

        // Scalar subtraction from each coordinate
        constexpr auto& operator-=(
            std::convertible_to<value_type> auto scalar) {
            std::ranges::transform(*this, this->begin(), [scalar](auto val) {
                return val - scalar;
            });
            return *this;
        }

        // Scalar multiplication to each coordinate
        constexpr auto& operator*=(
            std::convertible_to<value_type> auto scalar) {
            std::ranges::transform(*this, this->begin(), [scalar](auto val) {
                return val * scalar;
            });
            return *this;
        }

        // Scalar division from each coordinate
        constexpr auto& operator/=(
            std::convertible_to<value_type> auto scalar) {
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

    /*this is a matrix view which reinterprets a contiguous range as matrix*/
    template <typename T, typename LP>
    requires(std::same_as<LP, std::layout_right> ||
             std::same_as<LP, std::layout_left>)
    class matrix_view : public std::mdspan<T,
                                           std::dextents<std::size_t, 2>,
                                           LP,
                                           std::default_accessor<T>>,
                        public std::span<T, std::dynamic_extent> {

      private:
        using mdspan_type = std::mdspan<T,
                                        std::dextents<std::size_t, 2>,
                                        LP,
                                        std::default_accessor<T>>;
        using span_type = std::span<T, std::dynamic_extent>;
        using data_handle_type = typename mdspan_type::data_handle_type;

      public:
        constexpr matrix_view(T* data,
                              std::size_t rows,
                              std::size_t columns,
                              LP layout = layout::row)
            : mdspan_type{data, rows, columns},
              span_type{data, rows * columns} {};


        template <std::ranges::contiguous_range R, typename _LP>
        constexpr matrix_view(R& data,
                              std::size_t rows,
                              std::size_t columns,
                              _LP layout)
            : mdspan_type{data.data(), rows, columns},
              span_type{data.data(), rows * columns} {};


      public:
        using mdspan_type::extent;


      public:
        using mdspan_type::operator[];


        // depending on layout returns ith row (layout::row) or ith column
        // (layout::column)
        constexpr auto operator[](std::size_t i) {
            auto dim{this->extent(std::same_as<LP, std::layout_right> ? 1 : 0)};
            return numeric_view{std::ranges::subrange{
                this->begin() + i * dim, this->begin() + (i + 1) * dim}};
        }


      public:
        /*returns the shape of this matrix, that is a pair (number of rows,
         * number of columns)*/
        constexpr auto shape() const -> std::pair<std::size_t, std::size_t> {
            return {extent(0), extent(1)};
        }
    };

    template <typename T, typename LP>
    matrix_view(T*, std::size_t, std::size_t, LP) -> matrix_view<T, LP>;


    template <std::ranges::contiguous_range R, typename _LP>
    matrix_view(R&, std::size_t, std::size_t, _LP)
        -> matrix_view<std::ranges::range_value_t<R>, _LP>;


    /*return a transposition matrix view*/
    template <typename T, typename LP>
    requires(std::same_as<LP, std::layout_right> ||
             std::same_as<LP, std::layout_left>)
    inline constexpr auto transpose(matrix_view<T, LP> m) {
        if constexpr (std::same_as<LP, std::layout_right>) {
            return matrix_view{
                m.data(), m.extent(1), m.extent(0), layout::column};
        } else {
            return matrix_view{m.data(), m.extent(1), m.extent(0), layout::row};
        }
    }

}  // namespace ranges


/*Finds the maximum width of column when it is converted to string*/
template <typename T, typename LP>
auto column_widths(::ranges::matrix_view<T, LP> m) -> std::vector<std::size_t> {
    std::vector<std::size_t> widths{};
    for (std::size_t j = 0; j < m.extent(1); ++j) {
        std::size_t width{0zu};
        for (std::size_t i = 0; i < m.extent(0); ++i) {
            width = std::max(width, std::format("{}", m[i, j]).size());
        }
        widths.push_back(width);
    }
    return widths;
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
    // if constexpr (std::same_as<LP, std::layout_right>) {
    std::string out{(column_padding > 0) ? "\n" : ""};
    const auto cols_widths{column_widths(m)};
    for (std::size_t i = 0; i < m.extent(0); ++i) {
        for (std::size_t j = 0; j < m.extent(1); ++j) {
            std::string str{std::format("{}", m[i, j])};
            if (j + 1 < m.extent(1)) {
                out += (column_padding > 0)
                           ? std::format("{: ^{}}{}",
                                         str,
                                         cols_widths[j] + 2 * column_padding,
                                         column_separator)
                           : std::format("{}{}", m[i, j], column_separator);
            } else {
                out += (column_padding > 0)
                           ? std::format("{: ^{}}",
                                         str,
                                         cols_widths[j] + 2 * column_padding)
                           : std::format("{}", m[i, j]);
            }
        }
        if (i + 1 < m.extent(0)) { out += row_separator; }
    }
    return out;
}

/*formatter for matrix of the form
 * {:<column_separator><row_separator><column_padding_value>}*/
template <typename T, typename LP>
struct std::formatter<::ranges::matrix_view<T, LP>, char> {
    char column_separator{','};
    char row_separator{'\n'};
    std::size_t column_padding{1zu};
    template <class ParseContext>
    constexpr ParseContext::iterator parse(ParseContext& ctx) {
        auto pos = ctx.begin();
        std::size_t i{0};
        while (pos != ctx.end() && *pos != '}') {
            if (i == 0) { column_separator = *pos; }
            if (i == 1) { row_separator = *pos; }
            if (i == 2) {
                column_padding =
                    std::min(static_cast<std::size_t>(*pos - '0'), 4zu);
            }
            ++i;
            ++pos;
        }
        return pos;
        //return ctx.begin();
    }

    template <class FmtContext>
    FmtContext::iterator format(::ranges::matrix_view<T, LP> m,
                                FmtContext& ctx) const {
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
    template <char column_separator = ',',
              char row_separator = '\n',
              typename T,
              typename LP>
    inline auto save(ranges::matrix_view<T, LP> m, std::filesystem::path file)
        -> void {
        std::filesystem::create_directories(file.parent_path());
        if (std::FILE * stream{std::fopen(file.c_str(), "w")}) {
            std::print(stream,
                       "{}",
                       ::to_string(m, column_separator, row_separator, 0));
            std::fclose(stream);
        } else {
            std::print("failed to create file {}, reason = {}\n",
                       file.string(),
                       std::strerror(errno));
        }
    }

    /*given a string representing a matrix which separators are column_separator
    and row_separator finds its shape and returns a triple: a vector with values
    converted to the type T, number of rows, number of columns.*/
    template <typename T, char column_separator = ',', char row_separator = ';'>
    inline auto to_values_and_shape(std::string_view content)
        -> std::tuple<std::vector<T>, std::size_t, std::size_t> {
        auto row_seps{static_cast<std::size_t>(
            std::ranges::count(content, row_separator))};
        auto col_seps{static_cast<std::size_t>(
            std::ranges::count(content, column_separator))};

        if ((row_seps == 0zu) && (col_seps == 0zu)) {
            return {std::vector<T>{}, 0zu, 0zu};
        }
        std::size_t rows{row_seps + 1};
        std::size_t cols{col_seps / rows + 1};
        std::vector<T> values{};
        for (auto&& row :
             content | std::views::split(std::views::single(row_separator))) {
            std::ranges::copy(
                row | std::views::split(std::views::single(column_separator)) |
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
    template <typename T, char column_separator = ',', char row_separator = ';'>
    [[nodiscard]] inline auto load(std::filesystem::path file)
        -> std::pair<std::vector<T>,
                     ranges::matrix_view<T, std::layout_right>> {
        std::string content{utils::load<char>(file)};
        auto [matrix_values, rows, cols] =
            to_values_and_shape<T, column_separator, row_separator>(content);

        ::ranges::matrix_view m{matrix_values, rows, cols, layout::row};
        return {std::move(matrix_values), std::move(m)};
    }

}  // namespace matrix


void numeric_view_examples() {
    std::vector v{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    ranges::numeric_view nv{v};
    std::println("nv = {}\n", nv);
    nv += 1;
    std::println("[1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12] + 1 = {}\n", nv);
    nv += v;
    std::println("2 * ([1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12] + 1) = {}\n",
                 nv);
    nv *= 2;
    std::println("4 * ([1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12] + 1) = {}\n",
                 nv);
    (nv /= 4) -= 1;
    std::println("v = {}\n", nv);
}


template <typename Layout>
void matrix_view_examples(Layout matrix_layout) {
    std::string_view layout_name{
        std::same_as<Layout, std::layout_right> ? "row" : "column"};
    std::println("////////////// using {} layout //////////\n\n\n",
                 layout_name);
    std::vector v{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    ranges::matrix_view m{v, 3, 4, matrix_layout};
    std::println("matrix m extents = {}", std::pair{m.extent(0), m.extent(1)});
    std::println("matrix m shape (rows, cols) = {}", m.shape());
    std::println(
        "m = {:.\n}\n matrix m first {} = {}\n m at coordinate [0,2] = {}\n\n",
        m,
        layout_name,
        m[0],
        m[0, 2]);
    std::println("transpose of m = {}\n", transpose(m));
}


void matrix_view_save_load_example() {
    std::vector v{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    ranges::matrix_view m{v, 3, 4, layout::row};
    std::filesystem::path file{"./data/matrix.txt"};
    std::println("saving m = {} to the file {}\n", m, file.string());
    matrix::save<',', ';'>(m, file);
    std::println(
        "loading a matrix of ints with columns separated by , and rows by ; "
        "from file {}\n",
        file.string());
    auto [data, loaded_matrix] = matrix::load<int, ',', ';'>(file);
    std::println("loaded matrix = {}\n", loaded_matrix);
}

/*adds to row_i row_j multiplied by alpha*/
template <typename T, typename LP, typename R>
requires(std::same_as<LP, std::layout_right>)
inline auto add(ranges::matrix_view<T, LP> m,
                std::size_t row_i,
                std::size_t row_j,
                R alpha) -> void {
    m[row_j] *= alpha;
    m[row_i] += m[row_j];
    m[row_j] /= alpha;
}

/*subtracts row_j multiplied by alpha from row_i*/
template <typename T, typename LP, typename R>
requires(std::same_as<LP, std::layout_right>)
inline auto subtract(ranges::matrix_view<T, LP> m,
                std::size_t row_i,
                std::size_t row_j,
                R alpha) -> void {
    m[row_j] *= alpha;
    m[row_i] -= m[row_j];
    m[row_j] /= alpha;
}

/*swaps rows of a matrix*/
template <typename T, typename LP>
requires(std::same_as<LP, std::layout_right>)
inline auto swap(ranges::matrix_view<T, LP> m,
                 std::size_t row_i,
                 std::size_t row_j) -> void {
    std::ranges::swap_ranges(m[row_i], m[row_j]);
}


void operations_on_matrix_rows() {
    std::println("////////////// OPERATIONS ON ROWS //////////\n\n\n");
    std::vector v{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    ranges::matrix_view m{v, 3, 4, layout::row};
    std::println("matrix m = {}", m);
    m[0] -= m[1];
    std::println("after  m[0] -= m[1], m = {}\n", m);
    add(m, 2, 1, 3);
    std::println("after  adding to m[2],  3 * m[1],  m = {}\n", m);
    swap(m, 0, 1);
    std::println("after swapping of rows 0 and 1, m = {}\n", m);
    m[0] *= 10;
    std::println("after multiplication of first row by 10, m = {}\n", m);

    ;
}