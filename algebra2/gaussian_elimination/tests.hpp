#include "gaussian_elimination_main.hpp"

namespace algorithms::gaussian_elimination {

    /*
        description:
            examples of using run and show_steps functions implementing gaussian
       elimination operations on matrices
    */
    inline auto examples_run_and_steps() {
        std::vector v1{1, 1, -9, 6, 7, 10, 0, 0, -6, 4, 2, 3, 0, 0, -3, 2};
        std::vector v2{1, -3, -3, -1, 1, 2, 2, -3, -14};
        ranges::matrix_view m1(v1, 4, 4, layout::row);
        ranges::matrix_view m2(v2, 3, 3, layout::row);

        std::println("m1 = {}\n", m1);
        auto [reduced, m1_reduced] = run(m1);
        std::println("m1 reduced = {}\n", m1_reduced);
        std::println("Steps of reducing m2 to row echelon form:");
        show_steps(m2);
    }


    /*
        description:
            examples of using determinant and inverse functions implementing
       gaussian elimination operations on matrices
    */
    inline auto examples_determinant_and_inverse() {
        std::vector<fraction<int>> v3{{1, 2},
                                      {1, 2},
                                      {1, 2},
                                      {1, 2},
                                      {2, 4},
                                      {1, 3},
                                      {5, 3},
                                      {1, 2},
                                      {9, 4}};
        std::vector<fraction<int>> v4{{1, 3},
                                      {1, 5},
                                      {9, 2},
                                      {5, 6},
                                      {6, 7},
                                      {11, 14},
                                      {15, 10},
                                      {0, 1},
                                      {9, 1}};
        ranges::matrix_view m3(v3, 3, 3, layout::row);
        ranges::matrix_view m4(v4, 3, 3, layout::row);
        std::println("m3 = {}", m3);
        auto det = determinant(m3).value();
        std::println("Determinant of m3 = {}\n", det);
        std::println("m4 = {}\n", m4);
        auto [inverse_vector, inverse_m4] = inverse(m4).value();
        std::println("Inverse of matrix m4 = {}\n", inverse_m4);
    }


    /*
        description:
            testing of the function run which implements gaussian elimination on
       a matrix
    */
    inline auto run_test() {
        std::vector v2{1, 1, -9, 6, 7, 10, 0, 0, -6, 4, 2, 3, 0, 0, -3, 2};
        const std::vector<fraction<int>> expected_v2{{1, 1},
                                                     {1, 1},
                                                     {-9, 1},
                                                     {6, 1},
                                                     {0, 1},
                                                     {3, 1},
                                                     {63, 1},
                                                     {-42, 1},
                                                     {0, 1},
                                                     {0, 1},
                                                     {-262, 1},
                                                     {179, 1},
                                                     {0, 1},
                                                     {0, 1},
                                                     {0, 1},
                                                     {-13, 262}};
        ranges::matrix_view m2(v2, 4, 4, layout::row);

        std::println("m2 = {}\n", m2);
        auto [m2_reduced_vector, m2_reduced] = run(m2);
        std::println("reduced m2 = {}\n", m2_reduced);

        std::vector m2_red_vec = m2_reduced | std::ranges::to<std::vector>();
        assert((m2_red_vec == expected_v2));
    }


    /*
        description:
            testing of the function determinant which calculates determinant
       using gaussian elimination on a matrix
    */
    inline auto determinant_test() {
        std::vector v3{4, 10, 1, 3, 6, 7, 2, 4, 1, 6, 7, 2, 2,
                       5, 3,  6, 7, 9, 0, 7, 4, 2, 5, 4, 4};
        const fraction<int> expected_det{5043, 1};
        ranges::matrix_view m3(v3, 5, 5, layout::column);

        std::println("m3 = {}\n", m3);
        auto det = determinant(m3).value();
        std::println("Determinant of m3 = {}\n", det);
        assert((det == expected_det));
    }


    /*
        description:
            testing of the function inverse which finds inverse matrix using
       gaussian elimination
    */
    inline auto inverse_test() {
        std::vector<fraction<int>> v1{{4, 1},
                                      {3, 4},
                                      {5, 9},
                                      {5, 9},
                                      {4, 7},
                                      {5, 6},
                                      {2, 6},
                                      {3, 2},
                                      {4, 3}};
        const std::vector<fraction<int>> expected_v1{{246, 979},
                                                     {84, 979},
                                                     {-155, 979},
                                                     {700, 2937},
                                                     {-7784, 2937},
                                                     {13720, 8811},
                                                     {-324, 979},
                                                     {2898, 979},
                                                     {-942, 979}};
        ranges::matrix_view m1(v1, 3, 3, layout::row);

        std::println("m1 = {}\n", m1);
        auto [m1_inverse_vector, m1_inverse] = inverse(m1).value();

        std::println("m1 inverse = {}\n", m1_inverse);
        std::vector m1_inv_vec = m1_inverse | std::ranges::to<std::vector>();
        assert((m1_inv_vec == expected_v1));
    }


    /*
        description:
            testing of the function inverse on column layout which implements
       gaussian elimination on a matrix
    */
    inline auto column_layout_test() {
        std::vector<fraction<int>> v1_for_column{{4, 1},
                                                 {3, 4},
                                                 {5, 9},
                                                 {5, 9},
                                                 {4, 7},
                                                 {5, 6},
                                                 {2, 6},
                                                 {3, 2},
                                                 {4, 3}};
        const std::vector<fraction<int>> v1_column_expected{{246, 979},
                                                            {700, 2937},
                                                            {-324, 979},
                                                            {84, 979},
                                                            {-7784, 2937},
                                                            {2898, 979},
                                                            {-155, 979},
                                                            {13720, 8811},
                                                            {-942, 979}};
        ranges::matrix_view m1_column(v1_for_column, 3, 3, layout::column);

        std::println("m1 using column layout {}\n", m1_column);
        auto [m1_inverse_vector_column, m1_column_inverse] =
            inverse(m1_column).value();
        std::println("m1 inverse using column layout {}\n", m1_column_inverse);

        std::vector m1_clmn_inv =
            m1_column_inverse | std::ranges::to<std::vector>();
        assert((m1_clmn_inv == v1_column_expected));
    }
}  // namespace algorithms::gaussian_elimination
