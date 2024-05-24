#include "matrix.hpp"

namespace matrix {
	inline auto numeric_view_examples() -> void {
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
    inline auto matrix_view_examples(Layout matrix_layout) -> void {
        std::string_view layout_name{
            std::same_as<Layout, std::layout_right> ? "row" : "column"};
        std::println("////////////// using {} layout //////////\n\n\n",
                     layout_name);
        std::vector v{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
        ranges::matrix_view m{v, 3, 4, matrix_layout};
        std::println("matrix m extents = {}",
                     std::pair{m.extent(0), m.extent(1)});
        std::println("matrix m shape (rows, cols) = {}", m.shape());
        std::println(
            "m = {:.\n}\n matrix m first {} = {}\n m at coordinate [0,2] = "
            "{}\n\n",
            m,
            layout_name,
            m[0],
            m[0, 2]);
        std::println("transpose of m = {}\n", transpose(m));
    }


    inline auto matrix_view_save_load_example() -> void {
        std::vector v{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
        ranges::matrix_view m{v, 3, 4, layout::row};
        std::filesystem::path const file{"./data/matrix.txt"};
        std::println("saving m = {} to the file {}\n", m, file.string());
        matrix::save<',', ';'>(m, file);
        std::println(
            "loading a matrix of ints with columns separated by , and rows by "
            "; "
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

    /*swaps rows of a matrix*/
    template <typename T, typename LP>
    requires(std::same_as<LP, std::layout_right>)
    inline auto swap(ranges::matrix_view<T, LP> m,
                     std::size_t row_i,
                     std::size_t row_j) -> void {
        std::ranges::swap_ranges(m[row_i], m[row_j]);
    }


    inline auto operations_on_matrix_rows() -> void {
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
    }

}