module;
#include <iostream>
#include <valarray>
#include <vector>

export module matrix;

export template <typename T>
class matrix : public std::valarray<T> {
  private:
    using base_t = std::valarray<T>;

  private:
    std::size_t _rows{};
    std::size_t _cols{};

  public:
    explicit matrix(std::size_t rows, std::size_t cols, T initial_value)
        : base_t(initial_value, rows * cols), _rows{rows}, _cols{cols} {}

  public:
    auto number_of_rows() const { return _rows; }


    auto number_of_columns() const { return _cols; }


    auto shape() const {
        return std::pair{number_of_rows(), number_of_columns()};
    }

  public:
    auto operator[](std::size_t row, std::size_t col) -> T&{
        return base_t::operator[](row * _cols + col);
    }


    auto operator[](std::size_t row, std::size_t col) const -> const T&{
        return base_t::operator[](row * _cols + col);
    }


    auto row(std::size_t row) {
        return base_t::operator[](std::slice(row * _cols, _cols, 1));
    }


    auto row(std::size_t row) const {
        return base_t::operator[](std::slice(row * _cols, _cols, 1));
    }


    auto column(std::size_t col) {
        return base_t::operator[](std::slice(col, _rows, _cols));
    }


    auto column(std::size_t col) const {
        return base_t::operator[](std::slice(col, _rows, _cols));
    }


    auto diagonal() {
        return base_t::operator[](std::slice(0, _rows, _cols + 1));
    }


    auto diagonal() const {
        return base_t::operator[](std::slice(0, _rows, _cols + 1));
    }

  public:
    auto operator+=(const T& val) -> matrix<T>& {
        base_t::operator+=(val);
        return *this;
    }


    auto operator+=(const matrix<T>& m) -> matrix<T>& {
        assert(m.shape() == shape());
        base_t::operator+=(static_cast<const base_t&>(m));
        return *this;
    }
    // template <unsigned, unsigned>
    // friend class matrixStack;
};

// template <typename ...>
// struct first_type;
// template <typename T, typename ...Ts>
// struct first_type<T, Ts...> {
//     using type = T;
// };
// template <typename ...Ts>
// using first_t = first_type<Ts...>::type;

// template <typename ...Ts>
// concept AllSame = (std::same_as<first_t<Ts...>, Ts> && ...);

export namespace utils::matrix {

    template <typename T>
    inline auto transpose(const ::matrix<T>& m) -> ::matrix<T> {
        ::matrix<T> transposed{m.number_of_columns(), m.number_of_rows(), T{0}};
        for (std::size_t row{0}; row < m.number_of_rows(); ++row) {
            transposed.column(row) = m.row(row);
        }
        return transposed;
    }


    template <typename T>
    inline auto identity(std::size_t dimension) -> ::matrix<T> {
        ::matrix<T> m{dimension, dimension, T{0}};
        m.diagonal() = T{1};
        return m;
    }


    template <typename T>
    inline auto eye(const std::valarray<T>& diagonal) -> ::matrix<T> {
        const auto dimension{diagonal.size()};
        ::matrix<T> m{dimension, dimension, T{0}};
        m.diagonal() = diagonal;
        return m;
    }


    template <typename T>
    inline auto eye(const std::slice_array<T>& diagonal) -> ::matrix<T> {
        const auto dimension{diagonal.size()};
        ::matrix<T> m{dimension, dimension, T{0}};
        m.diagonal() = diagonal;
        return m;
    }


    template <typename... Ts>
    requires(sizeof...(Ts) > 0)
    inline auto eye(Ts... diagonal) -> ::matrix<std::common_type_t<Ts...>> {
        using T = std::common_type_t<Ts...>;
        constexpr auto dimension{sizeof...(Ts)};
        ::matrix<T> m{dimension, dimension, T{0}};
        m.diagonal() = std::valarray{diagonal...};
        return m;
    }

}  // namespace utils::matrix

// template <unsigned dim = 3, unsigned max = 8>
// class matrixStack {
//     std::valarray<T> stack;
//     unsigned count = 0;

//   public:
//     matrixStack() : stack(dim * dim * max) {}
//     void prT_all() const {
//         std::valarray<T> row(dim * count);
//         for (unsigned r = 0; r != dim; ++r)  // screen row
//         {
//             row = stack[std::gslice(r * dim, {count, dim}, {dim * dim, 1})];
//             for (unsigned i = 0; i != row.size(); ++i)
//                 std::cout << row[i] << ((i + 1) % dim ? " " : " │ ");
//             std::cout << '\n';
//         }
//     }
//     void push_back(matrix const& m) {
//         if (count < max) {
//             stack[std::slice(count * dim * dim, dim * dim, 1)] =
//                 m.data[std::slice(0, dim * dim, 1)];
//             ++count;
//         }
//     }
// };


// namespace ranges {}


/*Finds the maximum width of column when it is converted to string*/
template <typename T>
inline auto column_widths(const matrix<T>& m) -> std::vector<std::size_t> {
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
template <typename T>
auto to_string(matrix<T> m,
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
template <typename T>
struct std::formatter<matrix<T>, char> {
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
    auto format(matrix<T> m, FmtContext& ctx) const -> FmtContext::iterator {
        return std::format_to(
            ctx.out(), "{}", ::to_string(m, column_separator, row_separator));
    }
};
